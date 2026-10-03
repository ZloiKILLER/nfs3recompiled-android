#include <lib/gliderenderer.h>
#include <lib/renderer.h>
#include <lib/glidetmu.h>
#include <SDL3/SDL.h>
#include <lib/glcompat.h>
#include <lib/glfuncs.h>
#include <lib/glthread.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace win32
{

static const char g_glslPreamble[] = NFS_GLSL_PREAMBLE;


/* g_texCoord is s/w, t/w and 1/w; g_combine the colour and alpha combine
 * factors (0 or 1), the wrap/mipmap word and whether the corner is fogged;
 * g_atlasInfo the atlas size and the tile's size and place in it. */
static const char g_glVertexShader[] = ""
"in vec3 g_position;"
"in vec4 g_color;"
"in vec3 g_texCoord;"
"in vec4 g_combine;"
"in vec4 g_atlasInfo;"
"out vec3 v_texCoord;"
"out vec4 v_color;"
"out float v_fog;"
"flat out vec4 v_combine;"
"flat out vec4 v_atlasInfo;"
"uniform mat4 u_transform;"
/* Glide's fog table, a factor for each of 64 values of w, and the w of each
 * (fogW below).  A corner's factor is the table at its w, straight between the
 * two nearest entries -- worked out here rather than for every corner of every
 * triangle on the CPU, where the divide and the search were most of what
 * drawTriangle cost (2026-09-25).  The table can change between draws (four
 * times a frame in a race), so it goes with each draw call. */
"uniform float u_fogW[64];"
"uniform float u_fogTable[64];"
"float fogFactor(float oow)"
"{"
"    if (oow <= 0.0) return 0.0;"
"    float w = 1.0 / oow;"
"    if (w <= u_fogW[0]) return u_fogTable[0];"
"    if (w >= u_fogW[63]) return u_fogTable[63];"
/* Entry 4k is exactly 2^k, so w's exponent is the octave; the three entries
 * inside it are counted off.  1 <= w < 2^16 here. */
"    int octave = 4 * (int(floatBitsToUint(w) >> 23u) - 127);"
"    int lo = octave + int(w >= u_fogW[octave + 1]) + int(w >= u_fogW[octave + 2])"
"           + int(w >= u_fogW[octave + 3]);"
"    float span = u_fogW[lo + 1] - u_fogW[lo];"
"    float t = span > 0.0 ? (w - u_fogW[lo]) / span : 0.0;"
"    return u_fogTable[lo] + (u_fogTable[lo + 1] - u_fogTable[lo]) * t;"
"}"
"void main()"
"{"
"    v_texCoord = g_texCoord;"
"    v_combine = g_combine;"
"    v_color = g_color;"
"    v_atlasInfo = g_atlasInfo;"
"    v_fog = g_combine.q > 0.5 ? fogFactor(g_texCoord.p) : 0.0;"
"    gl_Position = u_transform * vec4(g_position, 1.0);"
"}";

static const char g_glFragmentShader[] =
"in vec3 v_texCoord;"
"in vec4 v_color;"
"flat in vec4 v_combine;"
"flat in vec4 v_atlasInfo;"
"in float v_fog;"
"layout (location=0) out vec4 o_color;"
"uniform sampler2D u_texture;"
/* The same atlas through a linear sampler, on unit 1 (NFS_GPU_FILTER). */
"uniform sampler2D u_linearTexture;"
"uniform vec3 u_fogColor;"
"uniform float u_alphaRef;"
"uniform bool u_dither;"
"uniform bool u_chromaKey;"
"uniform vec3 u_chromaKeyColor;"
"uniform float u_gamma;"
""
/* One texel of the atlas tile this triangle uses.  The atlas is sampled with
 * GL_NEAREST on purpose -- hardware filtering would bleed neighbouring tiles
 * into each other -- so wrap/clamp and the filter itself are done here, per
 * tile.  Sampling at texel+0.5 hits the texel centre, which is what makes the
 * NEAREST fetch land on exactly the texel asked for. */
/* v_combine.p packs three things: bit 0 wraps S, bit 1 wraps T, and
 * everything above is the number of mipmap levels this texture has. */
"vec2 wrapMask()"
"{"
"    return vec2(mod(v_combine.p, 2.0), mod(floor(v_combine.p * 0.5), 2.0));"
"}"
/* org, tile and atlas are the tile rectangle already scaled into the mipmap
 * level being sampled; a tile at (x, y) of size S sits at (x >> L, y >> L) of
 * size S >> L in atlas level L, exactly, because the allocator only ever
 * splits slots into aligned quadrants. */
"vec4 getTexel(vec2 texel, vec2 org, float tile, float atlas, float lod)"
"{"
"    vec2 wrapped = mod(texel, tile);"
"    vec2 clamped = clamp(texel, 0.0, tile - 1.0);"
"    vec2 t = mix(clamped, wrapped, wrapMask());"
"    return textureLod(u_texture, (org + t + 0.5) / atlas, lod);"
"}"
"void main()"
"{"
/* Bilinear, the way the Voodoo did it (the game asks for GR_TEXTUREFILTER_
 * BILINEAR and never anything else).  This used to be four fetches at +-0.25
 * texel averaged with constant 0.5 weights, which is not interpolation: with
 * NEAREST fetches both taps land on the same texel whenever the fractional
 * position is between 0.25 and 0.75, so the result was plain point sampling
 * for half of the surface and a hard 50/50 blend for the rest -- a three-step
 * staircase instead of a gradient, most visible on the big stretched alpha
 * textures like the headlight cones.  Same four fetches, real weights. */
"    vec2 texelPos = (v_texCoord.st/v_texCoord.p)*v_atlasInfo.t/(256.0);"
/* In clamp mode keep the whole 2x2 footprint inside the tile.  Letting it run
 * off the edge makes the filter blend the edge texel with a clamped copy of
 * itself, which lightens a one-pixel line along every tile border -- on a
 * picture drawn as a grid of tiled quads (the car loading screen) that reads
 * as a visible lattice.  Wrap mode must NOT be clamped: there the neighbour
 * genuinely comes from the opposite edge, which getTexel handles with mod. */
/* Pick the mipmap level the way the hardware would: from how fast the texture
 * coordinate moves across neighbouring pixels.  Without this everything is
 * sampled at full resolution however small it gets on screen, which looks
 * clean in a still frame and boils as soon as anything moves -- the reason
 * the artefacts on the light cones only show up in motion.  GR_MIPMAP_NEAREST
 * is what the game asks for, so round to the nearest level rather than
 * blending two of them. */
"    float maxLod = max(floor(v_combine.p * 0.25) - 1.0, 0.0);"
"    vec2 ddx = dFdx(texelPos);"
"    vec2 ddy = dFdy(texelPos);"
"    float rho = max(length(ddx), length(ddy));"
"    float lod = clamp(floor(log2(max(rho, 1e-6)) + 0.5), 0.0, maxLod);"
"    float scale = exp2(lod);"
"    vec2 org = floor(v_atlasInfo.pq / scale);"
"    float tile = max(v_atlasInfo.t / scale, 1.0);"
"    float atlas = v_atlasInfo.s / scale;"
"    vec2 pos = texelPos / scale;"
/* Bring a wrapped coordinate into the tile before anything else touches it.
 * The projected headlight texture produces very large coordinates, and every
 * step after this -- floor, the fractional weights, the per-tap mod -- keeps
 * only what precision is left in them.  Lose the fraction and the filter
 * collapses into flat blocks, which is what the mosaic in the lit area looks
 * like on Mali while Adreno, carrying more precision, shows nothing.  This
 * has to happen AFTER the derivatives above: wrapping first would put a seam
 * in the coordinate and make dFdx explode across it. */
"    pos = mix(pos, mod(pos, tile), wrapMask());"
/* In clamp mode keep the whole 2x2 footprint inside the tile.  Letting it run
 * off the edge makes the filter blend the edge texel with a clamped copy of
 * itself, which lightens a one-pixel line along every tile border -- on a
 * picture drawn as a grid of tiled quads (the car loading screen) that reads
 * as a visible lattice.  Wrap mode must NOT be clamped: there the neighbour
 * genuinely comes from the opposite edge, which getTexel handles with mod. */
"    vec2 clampedPos = clamp(pos, 0.5, max(tile - 0.5, 0.5));"
"    pos = mix(clampedPos, pos, wrapMask());"
"    vec2 base = floor(pos - 0.5);"
/* The GPU's own bilinear filter reads the four texels in one fetch, and gives
 * the same weights, wherever the 2x2 footprint lies inside the tile and no
 * texel of it is to be keyed out: nothing of the next tile can bleed in then.
 * It had been taken out on 2026-09-25 as the suspect for the moving mosaic in
 * the headlights' light, which turned out to be depth (NFS_DEPTH16 below); it
 * takes about a fifth of the GPU's work off a night race. */
"\n#ifdef NFS_GPU_FILTER\n"
"    vec4 textureColor;"
"    if (!u_chromaKey && all(greaterThanEqual(base, vec2(0.0))) && all(lessThanEqual(base, vec2(tile - 2.0)))) {"
"        textureColor = textureLod(u_linearTexture, (org + pos) / atlas, lod);"
"    } else {"
"\n#endif\n"
"    vec2 w = pos - 0.5 - base;"
"    vec4 t00 = getTexel(base,                    org, tile, atlas, lod);"
"    vec4 t10 = getTexel(base + vec2(1.0, 0.0),   org, tile, atlas, lod);"
"    vec4 t01 = getTexel(base + vec2(0.0, 1.0),   org, tile, atlas, lod);"
"    vec4 t11 = getTexel(base + vec2(1.0, 1.0),   org, tile, atlas, lod);"
/* Chroma-key is a per-texel test in the Glide texture pipeline, before
 * filtering.  Testing the filtered colour instead (what this did before)
 * only ever matches in the middle of a keyed area and leaves a halo of
 * half-keyed texels around every cut-out edge, so the keyed taps are dropped
 * from the weighted sum instead. */
"    vec4 keep = vec4(1.0);"
"    if (u_chromaKey) {"
"        keep.x = all(equal(t00.rgb, u_chromaKeyColor)) ? 0.0 : 1.0;"
"        keep.y = all(equal(t10.rgb, u_chromaKeyColor)) ? 0.0 : 1.0;"
"        keep.z = all(equal(t01.rgb, u_chromaKeyColor)) ? 0.0 : 1.0;"
"        keep.w = all(equal(t11.rgb, u_chromaKeyColor)) ? 0.0 : 1.0;"
"    }"
"    vec4 wgt = vec4((1.0 - w.x) * (1.0 - w.y), w.x * (1.0 - w.y),"
"                    (1.0 - w.x) * w.y,         w.x * w.y) * keep;"
"    float wsum = wgt.x + wgt.y + wgt.z + wgt.w;"
"    if (wsum <= 0.0) discard;"
"\n#ifdef NFS_GPU_FILTER\n"
"    textureColor = (t00 * wgt.x + t10 * wgt.y + t01 * wgt.z + t11 * wgt.w) / wsum;"
"    }"
"\n#else\n"
"    vec4 textureColor = (t00 * wgt.x + t10 * wgt.y"
"                        + t01 * wgt.z + t11 * wgt.w) / wsum;"
"\n#endif\n"
"    vec4 color = vec4(mix(v_color.rgb, v_color.rgb * textureColor.rgb, v_combine.s),"
"                      mix(v_color.a, v_color.a * textureColor.a, v_combine.t));"
"    if (color.a <= u_alphaRef) discard;"
/* Glide fogs the colour only; alpha is left alone. */
"    color.rgb = mix(color.rgb, u_fogColor, clamp(v_fog, 0.0, 1.0));"
/* The game's own gamma; at 1 the pow() is only a slower copy. */
"    if (u_gamma != 1.0)"
"        color.rgb = pow(max(color.rgb, vec3(0.0)), vec3(1.0 / max(u_gamma, 0.001)));"
"    if (u_dither) {"
"        const float bayer[16] = float[16](0.0, 8.0, 2.0, 10.0, 12.0, 4.0, 14.0, 6.0,"
"                                           3.0, 11.0, 1.0, 9.0, 15.0, 7.0, 13.0, 5.0);"
"        int bx = int(mod(gl_FragCoord.x, 4.0));"
"        int by = int(mod(gl_FragCoord.y, 4.0));"
"        float d = (bayer[by * 4 + bx] - 7.5) / 16.0;"
"        color.rgb += d * vec3(1.0 / 31.0, 1.0 / 63.0, 1.0 / 31.0);"
"    }"
"    o_color = color;"
/* The Voodoo's depth buffer held 16 bits: ooz's whole part, 0 to 65535, which
 * this projection makes gl_FragCoord.z * 65536.  A decal the game lays on the
 * road with LEQUAL -- the headlights' light pool, a car's shadow, the glow of
 * its lights in the rain -- came out the same 16 bits as the road under it and
 * passed.  In the finer depth Mali gives, the two surfaces' last bits differ
 * from pixel to pixel, and the decal flickers against the road and against
 * each other.  So each fragment lands in the middle of its 16-bit step, as on
 * the Voodoo.  Built only where the depth buffer has more than 16 bits and the
 * GPU is not an Adreno 5xx (needsDepthSteps); NFS_DEPTH16=0 or 1 decides
 * instead. */
"\n#ifdef NFS_DEPTH16\n"
"    gl_FragDepth = (floor(gl_FragCoord.z * 65536.0) + 0.5) / 65536.0;"
"\n#endif\n"
"}";

/* 40 bytes.  What is the same for the whole triangle -- the combine factors,
 * which are only ever 0 or 1, the wrap/mipmap word, whether it is fogged and
 * the tile in the atlas -- goes as bytes and shorts the GPU turns back into the
 * same floats: it used to be ten floats in each of the three corners, all
 * written here and all copied again by the driver at every draw.  The fog
 * factor itself is the vertex shader's, from 1/w. */
struct GlVertex
{
    float           x, y, z;
    x86::reg32      color;
    float           u, v, oow;
    std::uint16_t   atlasInfo[4];   // atlas size, tile size, tile x, tile y
    x86::reg8       combine[4];     // colour factor, alpha factor, wrap word, fogged
};
static_assert(sizeof(GlVertex) == 40, "the attribute offsets below assume this layout");

/* Glide's fog table is indexed by w, with four entries per octave.  This is
 * guFogTableIndexToW() from the Glide 2.x reference:
 *     w(i) = 2^(3 + i/4) / (8 - (i % 4))
 * so entry 0 is w=1 and entry 63 is w~52429.  Precomputed once because the
 * per-vertex lookup below searches it. */
static float s_fogW[64];
static bool  s_fogWReady = false;

static void initFogW()
{
    if (s_fogWReady)
        return;
    for (int i = 0; i < 64; ++i)
    {
        s_fogW[i] = float(pow(2.0, 3.0 + double(i >> 2)) / double(8 - (i & 3)));
    }
    s_fogWReady = true;
}

/* Vertices queued between two draws of a frame, sixty bytes each.  32000 held
 * a single-player race; split screen with every car on its detailed model
 * (NFS_CAR_DETAIL_FULL) queued more and wrote past the end of the array,
 * crashing a second or two into every split-screen race. */
static const x86::reg32 s_maxVertexCount = 128000;

/* The arrays vertices are queued in.  renderPending() hands the full one to the
 * GL thread with what it draws and goes on in another; the GL thread gives it
 * back once the driver has its copy.  Two or three in all while the GL thread
 * runs a frame behind, one where there is none. */
static std::mutex s_vertexPoolMutex;
static std::vector<GlVertex*> s_vertexPool;

static GlVertex* takeVertexArray()
{
    {
        std::lock_guard<std::mutex> lock(s_vertexPoolMutex);
        if (!s_vertexPool.empty())
        {
            GlVertex* vertices = s_vertexPool.back();
            s_vertexPool.pop_back();
            return vertices;
        }
    }
    return new GlVertex[s_maxVertexCount];
}

static void giveVertexArray(GlVertex* vertices)
{
    std::lock_guard<std::mutex> lock(s_vertexPoolMutex);
    s_vertexPool.push_back(vertices);
}

/* What renderPending() hands the GL thread: the queued vertices and batches,
 * and a copy of every setting of the game's the batches are drawn with. */
struct PendingDraw
{
    GlVertex*                           vertices;
    x86::reg32                          vertexCount;
    std::vector<DrawCall>               drawCalls;
    std::vector<std::array<float, 64>>  fogTables;
    float                               fogColor[3];
    float                               alphaTestRef;
    x86::reg32                          chromaKeyColor;
    x86::reg32                          width;
    x86::reg32                          height;
};

GlideCounters GlideRenderer::s_counters;

GlideRenderer::GlideRenderer(Renderer* renderer, x86::reg32 preferredAtlasSize)
    :   m_renderer(renderer)
    ,   m_vertices(takeVertexArray())
    ,   m_vertexCount(0)
    ,   m_shaderProgram(0)
    ,   m_shaderPrograms{0, 0}
    ,   m_depth16(true)
    ,   m_depthTurns(false)
    ,   m_gpuFilter(true)
    ,   m_linearSampler(0)
    ,   m_fogMode(0)
    ,   m_fogTableValid(false)
    ,   m_fogColorUniform(-1)
    ,   m_alphaTestRef(0.f)
    ,   m_alphaRefUniform(-1)
    ,   m_ditherUniform(-1)
    ,   m_chromaKeyUniform(-1)
    ,   m_chromaKeyColorUniform(-1)
    ,   m_gammaUniform(-1)
    ,   m_zBuffer(true)
    ,   m_zTest(true)
    ,   m_depthAlways(false)
    ,   m_alphaBlend(true)
    ,   m_dither(false)
    ,   m_cull(false)
    ,   m_chromaKey(false)
    ,   m_textureMipLevels(1)
    ,   m_renderStamp(1)
    ,   m_texWrapS(true)
    ,   m_texWrapT(true)
    ,   m_cullMode(0)
    ,   m_chromaKeyColor(0)
    ,   m_clipMinX(0)
    ,   m_clipMinY(0)
    ,   m_clipMaxX(0)
    ,   m_clipMaxY(0)
    ,   m_gamma(1.f)
    ,   m_textureUploadScratch(reinterpret_cast<x86::reg8*>(malloc(256*256*4)))
{
    initFogW();
    m_fogColor[0] = m_fogColor[1] = m_fogColor[2] = 0.f;
    m_fogTables.assign(1, std::array<float, 64>());   // no fog until the game sets a table
    m_fogTableIndex = 0;
    /* All of it GL work, and the atlas's size, which the tile allocator below
     * is made for, comes out of it: done on the GL thread and waited for. */
    glthread::post(m_renderer, [this, preferredAtlasSize]() { initGl(preferredAtlasSize); });
    glthread::finish();
    m_tmus[0] = new GlideTMU(m_atlasSize);
    //m_tmus[1] = new GlideTMU(m_atlasSize);
}

/* Whether the shader has to cut depth to the Voodoo's 16-bit steps itself.
 * Only where the driver gave more than the 16 bits asked for: Mali gives 24,
 * and there a decal and the road under it differ in their last bits and the
 * headlights' light flickers.  A buffer that really is 16 bits already holds
 * the Voodoo's steps, and the shader's own write would only cost the early
 * depth test.
 *
 * Never on an Adreno 5xx, whatever it reports.  On a Snapdragon 820 (Adreno
 * 530) the write broke the depth test outright: the wheels showed through the
 * body on the Player Car screen, where 0.75, without the write, drew them
 * right.  Drivers of that generation have known faults in depth written by the
 * shader together with discard, which this shader does, and Adreno never showed
 * the flicker the write is there for. */
bool needsDepthSteps(int depthBits, const char* renderer)
{
    if (renderer)
    {
        const char* adreno = SDL_strstr(renderer, "Adreno");
        if (adreno)
        {
            const char* model = adreno;
            while (*model && (*model < '0' || *model > '9'))
                ++model;
            if (model[0] == '5' && model[1] >= '0' && model[1] <= '9' && model[2] >= '0' && model[2] <= '9')
                return false;
        }
    }
    return depthBits > 16;
}

void GlideRenderer::initGl(x86::reg32 preferredAtlasSize)
{
    loadGlFunctions();
    /* Every texture the game uploads lives in this one atlas.  2048 is 64 whole
     * 256x256 tiles, what the game's own texture sizes were made for; asked for
     * more (every car at the player's texture size), take 4096 wherever the GPU
     * allows it -- GLES 3 only promises 2048. */
    GLint maxTextureSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    m_atlasSize = preferredAtlasSize >= 4096 && maxTextureSize >= 4096 ? 4096 : 2048;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[GFX] texture atlas %ux%u (asked for %u, GPU limit %d)",
                unsigned(m_atlasSize), unsigned(m_atlasSize), unsigned(preferredAtlasSize), int(maxTextureSize));

    glGenTextures(1, &m_atlas);
    glBindTexture(GL_TEXTURE_2D, m_atlas);
    /* NEAREST on both, with a mipmap min filter so textureLod() can reach
     * levels above 0: filtering and level choice are done by hand in the
     * shader, per tile, because hardware filtering would bleed one atlas
     * tile into the next.  Nine levels covers a 256-texel tile all the
     * way down to a single texel. */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexStorage2D(GL_TEXTURE_2D, 9, GL_RGBA8, GLsizei(m_atlasSize), GLsizei(m_atlasSize));
    /* The same atlas filtered by the GPU, for the shader's footprints that lie
     * inside their tile; the level is still the shader's choice (textureLod). */
    glGenSamplers(1, &m_linearSampler);
    glSamplerParameteri(m_linearSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glSamplerParameteri(m_linearSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(m_linearSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(m_linearSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);

    glGenVertexArrays(1, &m_vertexArray);
    glBindVertexArray(m_vertexArray);
    glGenBuffers(1, &m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);

    glGenRenderbuffers(1, &m_depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_renderer->m_width, m_renderer->m_height);

    glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderer->m_texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBuffer);
    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);

    /* What the driver actually gave us, not what was asked for.  The render
     * target is created with the unsized internal format GL_RGB, which
     * desktop GL and GLES are free to resolve differently -- 8 bits per
     * channel on one and 5/6/5 on the other would explain colour banding and
     * crawling dither that shows on the phone and not on Windows.  Logged
     * once at startup so both platforms can be compared directly. */
    {
        GLint r = 0, g = 0, b = 0, a = 0, d = 0;
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                              GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE, &r);
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                              GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE, &g);
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                              GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE, &b);
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                              GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE, &a);
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                              GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &d);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                    "[GLINFO] renderer=%s version=%s",
                    (const char*)glGetString(GL_RENDERER),
                    (const char*)glGetString(GL_VERSION));
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                    "[GLINFO] render target R%d G%d B%d A%d, depth %d бит",
                    r, g, b, a, d);
        m_depth16 = needsDepthSteps(d, (const char*)glGetString(GL_RENDERER));
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // After the depth buffer: which depth the shader writes follows from what it got.
    compileShaders();
}

GlideRenderer::~GlideRenderer()
{
    const GLuint framebuffer = m_framebuffer;
    glthread::post(m_renderer, [framebuffer]() { glDeleteFramebuffers(1, &framebuffer); });
    glthread::finish();
    giveVertexArray(m_vertices);
    delete m_tmus[0];
    free(m_textureUploadScratch);
}

/* Turns of the depth comparison (NFS_DEPTH16=ab), each this long. */
static const Uint64 s_depthTurnMs = 5000;

void GlideRenderer::compileShaders()
{
    GLuint vertexShader;
    const GLint vertexShaderLen[2] = { GLint(sizeof(g_glslPreamble) - 1),
                                       GLint(sizeof(g_glVertexShader) - 1) };
    const GLchar* vertexShaderSrc[2] = { g_glslPreamble, g_glVertexShader };
    GLint status = 0;
    /* m_depth16 comes in as initGl found it (needsDepthSteps).  NFS_DEPTH16=0
     * or 1 overrides that: the GPU's own depth, or the Voodoo's 16-bit steps
     * written by the shader.  A shader that writes gl_FragDepth loses the
     * GPU's early depth test, so NFS_DEPTH16=ab changes between the two every
     * few seconds, to weigh what the steps cost the GPU in the same race
     * ([TICKS] counts the frames of each). */
    const char* depth16Env = SDL_getenv("NFS_DEPTH16");
    m_depthTurns = depth16Env && SDL_strcmp(depth16Env, "ab") == 0;
    if (depth16Env && SDL_strcmp(depth16Env, "0") == 0)
        m_depth16 = false;
    else if (depth16Env && SDL_strcmp(depth16Env, "1") == 0)
        m_depth16 = true;
    /* NFS_GPU_FILTER=0: every texel through the four fetches of the shader's
     * own filter, to compare pictures with. */
    const char* gpuFilterEnv = SDL_getenv("NFS_GPU_FILTER");
    m_gpuFilter = !(gpuFilterEnv && SDL_strcmp(gpuFilterEnv, "0") == 0);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[GFX] texture filter: %s",
                m_gpuFilter ? "the GPU's where a texel's four lie in its tile" : "the shader's own");

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 2, vertexShaderSrc, vertexShaderLen);
    glCompileShader(vertexShader);

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        GLint maxLen = 0;
        GLsizei len = 0;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLen);
        if (maxLen > 1)
        {
            GLchar *log = (GLchar *)malloc(maxLen);
            glGetShaderInfoLog(vertexShader, maxLen, &len, log);
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "%s", log);
            free(log);
        }
    }
    for (int depth16 = 0; depth16 < 2; ++depth16)
        if (m_depthTurns || bool(depth16) == m_depth16)
            m_shaderPrograms[depth16] = linkProgram(vertexShader, bool(depth16));
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[GFX] depth in %s",
                m_depthTurns ? "turns of the Voodoo's 16-bit steps and the GPU's own precision, 5 s each"
                : m_depth16 ? "the Voodoo's 16-bit steps" : "the GPU's own precision");
    m_shaderProgram = m_shaderPrograms[m_depth16];
    locateShaderInputs();
}

GLuint GlideRenderer::linkProgram(GLuint vertexShader, bool depth16)
{
    std::string defines = depth16 ? "#define NFS_DEPTH16 1\n" : "";
    if (m_gpuFilter)
        defines += "#define NFS_GPU_FILTER 1\n";
    const char* define = defines.c_str();
    const GLint fragmentShaderLen[3] = { GLint(sizeof(g_glslPreamble) - 1), GLint(SDL_strlen(define)),
                                         GLint(sizeof(g_glFragmentShader) - 1) };
    const GLchar* fragmentShaderSrc[3] = { g_glslPreamble, define, g_glFragmentShader };
    GLint status = 0;
    const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 3, fragmentShaderSrc, fragmentShaderLen);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        GLint maxLen = 0;
        GLsizei len = 0;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLen);
        if (maxLen > 1)
        {
            GLchar *log = (GLchar *)malloc(maxLen);
            glGetShaderInfoLog(fragmentShader, maxLen, &len, log);
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "%s", log);
            free(log);
        }
    }

    const GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status)
    {
        GLint maxLen = 0;
        GLsizei len = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLen);
        if (maxLen > 1)
        {
            GLchar *log = (GLchar *)malloc(maxLen);
            glGetProgramInfoLog(program, maxLen, &len, log);
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "%s", log);
            free(log);
        }
    }
    // What stays the same for good: the atlas's unit and the fog table's w.
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "u_texture"), 0);
    glUniform1i(glGetUniformLocation(program, "u_linearTexture"), 1);
    initFogW();
    glUniform1fv(glGetUniformLocation(program, "u_fogW"), 64, s_fogW);
    glUseProgram(0);
    return program;
}

void GlideRenderer::locateShaderInputs()
{
    m_attributes[0] = glGetAttribLocation(m_shaderProgram, "g_position");
    m_attributes[1] = glGetAttribLocation(m_shaderProgram, "g_color");
    m_attributes[2] = glGetAttribLocation(m_shaderProgram, "g_texCoord");
    m_attributes[3] = glGetAttribLocation(m_shaderProgram, "g_combine");
    m_attributes[4] = glGetAttribLocation(m_shaderProgram, "g_atlasInfo");
    m_attributes[5] = -1;
    m_fogTableUniform = glGetUniformLocation(m_shaderProgram, "u_fogTable");
    m_transform = glGetUniformLocation(m_shaderProgram, "u_transform");
    m_fogColorUniform = glGetUniformLocation(m_shaderProgram, "u_fogColor");
    m_alphaRefUniform = glGetUniformLocation(m_shaderProgram, "u_alphaRef");
    m_ditherUniform = glGetUniformLocation(m_shaderProgram, "u_dither");
    m_chromaKeyUniform = glGetUniformLocation(m_shaderProgram, "u_chromaKey");
    m_chromaKeyColorUniform = glGetUniformLocation(m_shaderProgram, "u_chromaKeyColor");
    m_gammaUniform = glGetUniformLocation(m_shaderProgram, "u_gamma");
}

void GlideRenderer::takeDepthTurn()
{
    ++s_counters.frames;
    s_counters.depth16Frames += m_depth16;
    if (!m_depthTurns)
        return;
    const bool depth16 = SDL_GetTicks() / s_depthTurnMs % 2 == 0;
    if (depth16 == m_depth16)
        return;
    /* renderPending() sets every uniform again on each frame, so the other
     * program needs nothing carried over.  The program and where its inputs
     * are belong to the GL thread. */
    m_depth16 = depth16;
    glthread::post(m_renderer, [this, depth16]() {
        m_shaderProgram = m_shaderPrograms[depth16];
        locateShaderInputs();
    });
}

void GlideRenderer::clear(x86::reg32 color)
{
    /* Diagnostic for the rear-view mirror investigation: does grBufferClear
     * ever run with a clip window narrower than the full frame?  Remove once
     * the mirror is confirmed fixed. */
#ifdef NFS_TRACE_MSG
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "[MIRROR] grBufferClear color=0x%06x clip=%u,%u->%u,%u",
                color, m_clipMinX, m_clipMinY, m_clipMaxX, m_clipMaxY);
#endif
    /* In the GL thread's queue where it was called: before the vertices queued
     * so far are drawn, exactly as it ran before there was a queue. */
    const x86::reg32 clipMinX = m_clipMinX, clipMinY = m_clipMinY, clipMaxX = m_clipMaxX, clipMaxY = m_clipMaxY;
    glthread::post(m_renderer, [this, color, clipMinX, clipMinY, clipMaxX, clipMaxY]() {
        clearFrame(color, clipMinX, clipMinY, clipMaxX, clipMaxY);
    });
}

void GlideRenderer::clearFrame(x86::reg32 color, x86::reg32 clipMinX, x86::reg32 clipMinY,
                               x86::reg32 clipMaxX, x86::reg32 clipMaxY)
{
    glDepthMask(true);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderer->m_texture, 0);
    /* Real Glide hardware clears only the current clipping rectangle
     * (grClipWindow), not the whole buffer -- this is exactly how a
     * rear-view mirror gets drawn: clip to the mirror's screen rectangle,
     * clear just that area to the sky colour, then render the backward-
     * facing scene into it.  Clearing unconditionally here meant that clip
     * region was never actually isolated: an unscissored clear either wiped
     * out geometry the mirror pass needed to keep, or -- more likely, given
     * the mirror shows nothing but flat sky -- painted the whole frame with
     * the clear colour immediately before whatever draws the reflection,
     * with no visible difference from "the mirror is just empty". */
    if (clipMaxX > clipMinX && clipMaxY > clipMinY)
    {
        glEnable(GL_SCISSOR_TEST);
        glScissor(clipMinX, clipMinY,
                  clipMaxX - clipMinX, clipMaxY - clipMinY);
    }
    else
    {
        glDisable(GL_SCISSOR_TEST);
    }
    glClearColor(float(color >> 16 & 0xff)/255.0f, float(color >> 8 & 0xff)/255.0f, float(color >> 0 & 0xff)/255.0f, 0.0f);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GlideRenderer::render(x86::reg32 buffer)
{
    m_renderer->m_currentBuffer = buffer;
}

void GlideRenderer::flush()
{
    /* A state that changed and changed again with no triangle in between
     * leaves nothing to draw with it: a batch of none would only cost the GL
     * calls that set its state. */
    if (m_vertexCount == (m_drawCalls.empty() ? 0 : m_drawCalls.back().lastTriangle))
    {
        ++s_counters.emptyCalls;
        return;
    }
    ++s_counters.drawCalls;
    DrawCall dcall;
    dcall.zTest = m_zTest;
    dcall.zTestAlways = m_depthAlways;
    dcall.zBuffer = m_zBuffer;
    dcall.alphaBlend = m_alphaBlend;
    dcall.dither = m_dither;
    dcall.cull = m_cull;
    dcall.chromaKey = m_chromaKey;
    dcall.cullMode = m_cullMode;
    dcall.clipMinX = m_clipMinX;
    dcall.clipMinY = m_clipMinY;
    dcall.clipMaxX = m_clipMaxX;
    dcall.clipMaxY = m_clipMaxY;
    dcall.gamma = m_gamma;
    dcall.fogTable = m_fogTableIndex;
    dcall.lastTriangle = m_vertexCount;
#ifdef NFS_TRACE_MSG
    /* Walks every vertex of the batch and formats a line, per batch, with
     * a few hundred batches a frame -- fine for an investigation on the
     * desktop, not something to leave switched on in a shipped build. */
    const x86::reg32 first = m_drawCalls.empty() ? 0 : m_drawCalls.back().lastTriangle;
    if (dcall.lastTriangle > first)
    {
        /* Diagnostic for the rear-view mirror investigation.  Remove once
         * the mirror is confirmed fixed.
         *
         * The z range is the point of this pass: the game never issues a
         * grBufferClear under the mirror clip rectangle (verified over a full
         * race), so the mirror content depth-tests with LEQUAL against
         * whatever the main scene left in those pixels earlier in the same
         * frame.  Comparing the mirror batch range against the main-scene
         * range is what says whether it loses that test -- everything else
         * about this bug has been guesswork so far. */
        float zMin = 0.f, zMax = 0.f;
        float xMin = 0.f, xMax = 0.f, yMin = 0.f, yMax = 0.f;
        /* Iterated alpha and the atlas tile are the last two things that can
         * make a batch invisible while every state flag looks right: alpha at
         * or below the alpha-test reference is discarded outright, and a tile
         * that never got uploaded samples as nothing. */
        unsigned aMin = 255, aMax = 0;
        bool oneTile = true;
        for (x86::reg32 i = first; i < dcall.lastTriangle; ++i)
        {
            const float z = m_vertices[i].z;
            const float x = m_vertices[i].x;
            const float y = m_vertices[i].y;
            const unsigned a = unsigned(m_vertices[i].color >> 24) & 0xffu;
            if (i == first || z < zMin) zMin = z;
            if (i == first || z > zMax) zMax = z;
            if (i == first || x < xMin) xMin = x;
            if (i == first || x > xMax) xMax = x;
            if (i == first || y < yMin) yMin = y;
            if (i == first || y > yMax) yMax = y;
            if (a < aMin) aMin = a;
            if (a > aMax) aMax = a;
            if (m_vertices[i].offX != m_vertices[first].offX
             || m_vertices[i].offY != m_vertices[first].offY
             || m_vertices[i].width != m_vertices[first].width)
                oneTile = false;
        }
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                    "[MIRROR] flush vtx=%u..%u clip=%u,%u->%u,%u zTest=%d zWrite=%d cull=%d/%u alphaBlend=%d chroma=%d z=%.1f..%.1f xy=%.0f,%.0f..%.0f,%.0f a=%u..%u aRef=%.3f aComb=%.0f tile=%.0f,%.0f/%.0f%s",
                    first, dcall.lastTriangle, dcall.clipMinX, dcall.clipMinY, dcall.clipMaxX, dcall.clipMaxY,
                    dcall.zTest, dcall.zBuffer, dcall.cull, dcall.cullMode, dcall.alphaBlend, dcall.chromaKey,
                    double(zMin), double(zMax),
                    double(xMin), double(yMin), double(xMax), double(yMax),
                    aMin, aMax, double(m_alphaTestRef),
                    double(m_vertices[first].alphaCombine),
                    double(m_vertices[first].offX), double(m_vertices[first].offY),
                    double(m_vertices[first].width), oneTile ? "" : " (тайлов>1)");
    }
#endif
    m_drawCalls.push_back(dcall);
}

/* Everything queued since the last call, drawn into the Glide framebuffer.
 * Split out of swap() because a texture upload can now need it mid-frame:
 * the atlas tile a triangle uses is stored in its vertices, and those are
 * not drawn until the frame ends, so a slot reused in between would make
 * already-queued triangles sample the new texture.  On real Glide the
 * triangles were rasterised as they were submitted and saw the old
 * contents. */
void GlideRenderer::renderPending()
{
    if (!m_vertexCount)
        return;
    flush();
    /* Everything the draw needs goes with it; the vertex array itself changes
     * hands, and queuing goes on in another. */
    std::shared_ptr<PendingDraw> draw = std::make_shared<PendingDraw>();
    draw->vertices = m_vertices;
    draw->vertexCount = m_vertexCount;
    draw->drawCalls.swap(m_drawCalls);
    draw->fogTables = m_fogTables;
    std::copy(m_fogColor, m_fogColor + 3, draw->fogColor);
    draw->alphaTestRef = m_alphaTestRef;
    draw->chromaKeyColor = m_chromaKeyColor;
    draw->width = m_renderer->m_width;
    draw->height = m_renderer->m_height;
    m_vertices = takeVertexArray();
    glthread::post(m_renderer, [this, draw]() {
        drawPending(*draw);
        giveVertexArray(draw->vertices);
    });
    m_vertexCount = 0;
    // Only the table in force carries over to what is drawn next.
    m_fogTables.front() = m_fogTables[m_fogTableIndex];
    m_fogTables.resize(1);
    m_fogTableIndex = 0;
    ++m_renderStamp;
}

/* renderPending()'s GL work, on the GL thread. */
void GlideRenderer::drawPending(const PendingDraw& draw)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderer->m_texture, 0);
    {
        /* Renderer::present() binds its own vertex array for the blit, so this
         * one can no longer rely on the binding made in the constructor. */
        glBindVertexArray(m_vertexArray);
        glViewport(0, 0, draw.width, draw.height);
        glUseProgram(m_shaderProgram);
        /* This used to be glOrtho(0, w, 0, h, 0, -65536) read back with
         * glGetFloatv(GL_PROJECTION_MATRIX).  Neither the fixed-function matrix
         * stack nor that query exists in GLES, and the shader already takes the
         * matrix through u_transform, so build it here instead.  Column-major,
         * exactly what glOrtho(l=0, r=w, b=0, t=h, n=0, f=-65536) produced:
         * diagonal 2/(r-l), 2/(t-b), -2/(f-n) and translation -(r+l)/(r-l),
         * -(t+b)/(t-b), -(f+n)/(f-n), which collapses to -1 on every axis. */
        const float w = float(draw.width);
        const float h = float(draw.height);
        const float matrix[16] = {
            2.0f / w, 0.0f,     0.0f,           0.0f,
            0.0f,     2.0f / h, 0.0f,           0.0f,
            0.0f,     0.0f,     2.0f / 65536.0f, 0.0f,
            -1.0f,    -1.0f,    -1.0f,          1.0f,
        };
        glUniformMatrix4fv(m_transform, 1, GL_FALSE, matrix);
        if (m_gpuFilter)
        {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_atlas);
            glBindSampler(1, m_linearSampler);
            glActiveTexture(GL_TEXTURE0);
        }
        glBindTexture(GL_TEXTURE_2D, m_atlas);
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GlVertex)*draw.vertexCount, draw.vertices, GL_STREAM_DRAW);
        glVertexAttribPointer(m_attributes[0], 3, GL_FLOAT, GL_FALSE, sizeof(GlVertex),
                              (const void*)offsetof(GlVertex, x));
        glEnableVertexAttribArray(m_attributes[0]);
        glVertexAttribPointer(m_attributes[1], 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(GlVertex),
                              (const void*)offsetof(GlVertex, color));
        glEnableVertexAttribArray(m_attributes[1]);
        glVertexAttribPointer(m_attributes[2], 3, GL_FLOAT, GL_FALSE, sizeof(GlVertex),
                              (const void*)offsetof(GlVertex, u));
        glEnableVertexAttribArray(m_attributes[2]);
        // Not normalised: the bytes come through as the whole numbers they hold.
        glVertexAttribPointer(m_attributes[3], 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(GlVertex),
                              (const void*)offsetof(GlVertex, combine));
        glEnableVertexAttribArray(m_attributes[3]);
        glVertexAttribPointer(m_attributes[4], 4, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(GlVertex),
                              (const void*)offsetof(GlVertex, atlasInfo));
        glEnableVertexAttribArray(m_attributes[4]);
        glUniform3f(m_fogColorUniform, draw.fogColor[0], draw.fogColor[1], draw.fogColor[2]);
        glUniform1f(m_alphaRefUniform, draw.alphaTestRef);
        glUniform3f(m_chromaKeyColorUniform, float(draw.chromaKeyColor >> 16 & 0xff) / 255.f,
                    float(draw.chromaKeyColor >> 8 & 0xff) / 255.f,
                    float(draw.chromaKeyColor & 0xff) / 255.f);
        x86::reg32 first = 0;
        int fogTable = -1;
        /* Each batch sets only the state that differs from the batch before:
         * a split-screen race draws a few hundred batches a frame, and a dozen
         * GL calls for each was driver time on the game thread.  The first
         * sets all of it, as nothing is known of what came before. */
        const DrawCall* previous = nullptr;
        bool scissor = false;
        for (std::vector<DrawCall>::const_iterator it = draw.drawCalls.begin(); it != draw.drawCalls.end(); ++it)
        {
            const DrawCall& call = *it;
            if (call.fogTable != fogTable)
            {
                fogTable = call.fogTable;
                glUniform1fv(m_fogTableUniform, 64, draw.fogTables[fogTable].data());
            }
            if (!previous || call.zTest != previous->zTest)
            {
                if (call.zTest)
                    glEnable(GL_DEPTH_TEST);
                else
                    glDisable(GL_DEPTH_TEST);
            }
            /* GR_CMP_ALWAYS (grDepthBufferFunction) still writes depth per
             * grDepthMask on real hardware -- it is not the same as disabling
             * the test.  See setDepthAlways()'s header comment; this is what
             * makes the rear-view mirror's clipped "always passes, write fresh
             * depth" background pass work instead of leaving the main scene's
             * depth in place.  Only read while the test is on. */
            if (call.zTest && (!previous || !previous->zTest || call.zTestAlways != previous->zTestAlways))
                glDepthFunc(call.zTestAlways ? GL_ALWAYS : GL_LEQUAL);
            if (!previous || call.zBuffer != previous->zBuffer)
                glDepthMask(call.zBuffer ? GL_TRUE : GL_FALSE);
            if (!previous || call.alphaBlend != previous->alphaBlend)
            {
                if (call.alphaBlend)
                    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
                else
                    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
            }
            if (!previous || call.dither != previous->dither)
                glUniform1i(m_ditherUniform, call.dither ? 1 : 0);
            if (!previous || call.chromaKey != previous->chromaKey)
                glUniform1i(m_chromaKeyUniform, call.chromaKey ? 1 : 0);
            if (!previous || call.gamma != previous->gamma)
                glUniform1f(m_gammaUniform, call.gamma);
            if (!previous || call.cull != previous->cull)
            {
                if (call.cull)
                {
                    glEnable(GL_CULL_FACE);
                    glCullFace(GL_BACK);
                }
                else
                {
                    glDisable(GL_CULL_FACE);
                }
            }
            if (call.cull && (!previous || !previous->cull || call.cullMode != previous->cullMode))
                glFrontFace(call.cullMode == 2 ? GL_CW : GL_CCW);
            const bool clipped = call.clipMaxX > call.clipMinX && call.clipMaxY > call.clipMinY;
            if (!previous || clipped != scissor)
            {
                if (clipped)
                    glEnable(GL_SCISSOR_TEST);
                else
                    glDisable(GL_SCISSOR_TEST);
            }
            /* No Y flip.  The Glide projection maps guest y straight to GL y
             * (y_ndc = 2*y/h - 1), so a guest rectangle at y 7..110 really does
             * occupy GL rows 7..110 -- the flip to screen orientation happens
             * later, in the blit quad in Renderer::present().  Flipping the
             * scissor too put it at rows 658..761, the opposite edge of the
             * buffer, not overlapping the geometry at all, so everything drawn
             * under a narrow clip window was cut away.  Harmless for the
             * full-screen clip the game uses everywhere else, which is why
             * only the rear-view mirror was affected. */
            if (clipped && (!previous || !scissor || call.clipMinX != previous->clipMinX
                            || call.clipMinY != previous->clipMinY || call.clipMaxX != previous->clipMaxX
                            || call.clipMaxY != previous->clipMaxY))
                glScissor(call.clipMinX, call.clipMinY, call.clipMaxX - call.clipMinX, call.clipMaxY - call.clipMinY);
            scissor = clipped;
            previous = &call;
            glDrawArrays(GL_TRIANGLES, first, call.lastTriangle - first);
            first = call.lastTriangle;
        }
    }
}

void GlideRenderer::swap()
{
    /* This used to query the display mode and set the swap interval to
     * refresh/60 on every single frame -- two EGL round trips per frame, and it
     * silently overrode the 30 Hz pacing Renderer negotiated.  One shared
     * negotiation, cached after the first call. */
    m_renderer->ensureFramePacing();
    renderPending();
    takeDepthTurn();
    /* The frame is queued for the GPU; wait for its time here, so the GPU
     * draws it meanwhile. */
    m_renderer->paceFrame();
    glthread::post(m_renderer, [this]() {
        /* The presentation clear and blit must not inherit the last Glide
         * clip.  No glFlush() here: present() below runs on the same context,
         * and GL commands within one context already execute in submission
         * order; the flush present() does before SDL_GL_SwapWindow is the one
         * that matters for frame pacing. */
        glDisable(GL_SCISSOR_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(0);
        glBindTexture(GL_TEXTURE_2D, m_renderer->m_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    });
    m_renderer->present();
    m_vertexCount = 0;
}

x86::reg32 GlideRenderer::textureMemStart(x86::reg32 tmu)
{
    return m_tmus[tmu]->textureMemStart() + sizeof(GlTextureSlot*);
}

x86::reg32 GlideRenderer::textureMemEnd(x86::reg32 tmu)
{
    return m_tmus[tmu]->textureMemEnd();
}

void GlideRenderer::atlasFreeSpace(x86::reg32& texels, x86::reg32& wholeTiles, x86::reg32& total) const
{
    m_tmus[0]->freeSpace(texels, wholeTiles);
    total = m_tmus[0]->atlasTexels();
}

x86::reg32 GlideRenderer::getTextureMemSize(x86::reg32 /*largeMipmapSize*/, x86::reg32 /*smallMipmapSize*/, TextureFormat /*format*/)
{
    return sizeof(GlTextureSlot*);
}

void GlideRenderer::setZWrite(bool enable)
{
    if (enable != m_zBuffer)
    {
#ifdef NFS_TRACE_MSG
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[MIRROR] setZWrite %d -> %d, vtx=%u",
                    m_zBuffer, enable, m_vertexCount);
#endif
        flush();
        m_zBuffer = enable;
    }
}

void GlideRenderer::setZTest(bool enable)
{
    if (enable != m_zTest)
    {
#ifdef NFS_TRACE_MSG
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[MIRROR] setZTest %d -> %d, vtx=%u",
                    m_zTest, enable, m_vertexCount);
#endif
        flush();
        m_zTest = enable;
    }
}

void GlideRenderer::setDepthAlways(bool always)
{
    if (always != m_depthAlways)
    {
        flush();
        m_depthAlways = always;
    }
}

void GlideRenderer::setAlphaBlendDst(AlphaBlend method)
{
    if (m_alphaBlend != (method == AB_1mSrcAlpha))
    {
        flush();
        m_alphaBlend = method == AB_1mSrcAlpha;
    }
}

void GlideRenderer::setColorFactor(float colorFactor)
{
    m_colorFactor = colorFactor;
}

void GlideRenderer::setAlphaFactor(float alphaFactor)
{
    m_alphaFactor = alphaFactor;
}

void GlideRenderer::setTexture(x86::reg32 tmu, x86::reg32 address)
{
    NFS2_ASSERT(tmu == 0);
    GlTextureSlot** info = m_tmus[tmu]->getTextureInfo(address);
    /* A texture the atlas had no room for at any size (setTextureData) has no
     * slot: sample the one-texel texture uploaded when the window opened rather
     * than follow a null pointer. */
    if (!*info)
        info = m_tmus[tmu]->getTextureInfo(0);
    if (!*info)
        return;
    m_textureOffsetW = 256 >> (*info)->lod;
    m_textureOffsetX = (*info)->x;
    m_textureOffsetY = (*info)->y;
    m_textureMipLevels = (*info)->mipLevels;
    (*info)->useStamp = m_renderStamp;
}

void GlideRenderer::setTextureData(x86::reg32 tmu, x86::reg32 address, const void* data,
                                   x86::reg32 largeMipmap, x86::reg32 smallMipmap, TextureFormat format)
{
    NFS2_ASSERT(tmu == 0);
    x86::reg32 largeMipmapSize = 256 >> largeMipmap;
    x86::reg32 smallMipmapSize = 256 >> smallMipmap;

    GlTextureSlot** info = m_tmus[tmu]->getTextureInfo(address);
    if (*info)
    {
        if ((*info)->useStamp == m_renderStamp && m_vertexCount)
        {
            /* Queued triangles still point at this tile.  Draw them before
             * the contents change, otherwise they show the replacement --
             * a square of the wrong texture for one frame.  Rare by
             * construction, so the usual one-pass batching is untouched;
             * counted so it is visible if it ever becomes common. */
            static unsigned hits = 0;
            if (++hits <= 4u || (hits % 512u) == 0u)
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                            "[GFX] texture slot reused with %u pending vertices"
                            " (case %u)", m_vertexCount, hits);
            renderPending();
        }
        m_tmus[tmu]->returnTextureSlot(*info);
    }
    *info = m_tmus[tmu]->reserveTextureSlot(largeMipmap);
    /* The levels lie one after another, each a quarter of the one before, at
     * two bytes a texel -- four for the full-colour format. */
    const x86::reg32 texelBytes = format == TF_ARGB_8888 ? 4 : 2;
    /* No room at this size: keep the texture from a smaller level of its own
     * mipmap chain instead, a little softer.  This used to take the null slot
     * straight to the next line and crash the game. */
    const x86::reg32 askedSize = largeMipmapSize;
    while (!*info && largeMipmapSize > smallMipmapSize)
    {
        data = static_cast<const x86::reg8*>(data) + largeMipmapSize * largeMipmapSize * texelBytes;
        largeMipmapSize >>= 1;
        *info = m_tmus[tmu]->reserveTextureSlot(++largeMipmap);
    }
    if (!*info || largeMipmapSize != askedSize)
    {
        static unsigned shortfalls = 0;
        if (++shortfalls <= 8u || (shortfalls % 256u) == 0u)
        {
            if (*info)
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                            "[GFX] texture atlas full: %ux%u texture stored at %ux%u (case %u)",
                            unsigned(askedSize), unsigned(askedSize),
                            unsigned(largeMipmapSize), unsigned(largeMipmapSize), shortfalls);
            else
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                            "[GFX] texture atlas full: %ux%u texture dropped (case %u)",
                            unsigned(askedSize), unsigned(askedSize), shortfalls);
        }
        if (!*info)
            return;
    }

    x86::reg32 x = (*info)->x;
    x86::reg32 y = (*info)->y;
    /* Pixels are written as four bytes in R,G,B,A memory order and uploaded as
     * GL_UNSIGNED_BYTE.  The previous code packed a uint32 as 0xRRGGBBAA and
     * relied on GL_UNSIGNED_INT_8_8_8_8, which does not exist in GLES and is
     * endian-sensitive; byte writes are exact on both.
     *
     * Every level goes into one buffer, converted here -- the game's copy is in
     * guest memory, which it goes on to reuse -- and the GL thread uploads them
     * from it in the order they lie in. */
    NFS2_ASSERT(largeMipmapSize <= 256);
    const x86::reg32 topSize = largeMipmapSize;
    size_t texels = 0;
    for (x86::reg32 size = largeMipmapSize; size >= smallMipmapSize && size; size >>= 1)
        texels += size_t(size) * size;
    std::vector<x86::reg8> converted(texels * 4);
    x86::reg8* textureData = converted.data();
    x86::reg32 lod = 0;
    for (; largeMipmapSize >= smallMipmapSize; ++lod, largeMipmapSize >>= 1)
    {
        const x86::reg16* pixelData = reinterpret_cast<const x86::reg16*>(data);
        const x86::reg8* texelBytesData = static_cast<const x86::reg8*>(data);
        for (x86::reg32 i = 0; i < largeMipmapSize*largeMipmapSize; ++i)
        {
            x86::reg8* px = textureData + i*4;
            if (format == TF_ARGB_8888)
            {
                // 0xAARRGGBB little-endian: B, G, R, A in memory.
                const x86::reg8* t = texelBytesData + i*4;
                px[0] = t[2];
                px[1] = t[1];
                px[2] = t[0];
                px[3] = t[3];
                continue;
            }
            const x86::reg16 p = pixelData[i];
            switch(format)
            {
            case TF_RGB_565:
                px[0] = x86::reg8((p & 0xf800) >> 8);
                px[1] = x86::reg8((p & 0x07e0) >> 3);
                px[2] = x86::reg8((p & 0x001f) << 3);
                px[3] = 0xff;
                break;
            case TF_ARGB_1555:
                px[0] = x86::reg8(((p & 0x7c00) >> 10) * 0xff / 0x1f);
                px[1] = x86::reg8(((p & 0x03e0) >> 5) * 0xff / 0x1f);
                px[2] = x86::reg8(((p & 0x001f) >> 0) * 0xff / 0x1f);
                px[3] = x86::reg8((p & 0x8000) ? 0xff : 0x00);
                break;
            case TF_ARGB_4444:
                {
                    const x86::reg8 r = x86::reg8(p >> 8  & 0xf);
                    const x86::reg8 g = x86::reg8(p >> 4  & 0xf);
                    const x86::reg8 b = x86::reg8(p       & 0xf);
                    const x86::reg8 a = x86::reg8(p >> 12 & 0xf);
                    px[0] = x86::reg8(r << 4 | r);
                    px[1] = x86::reg8(g << 4 | g);
                    px[2] = x86::reg8(b << 4 | b);
                    px[3] = x86::reg8(a << 4 | a);
                    break;
                }
            default:
                px[0] = px[1] = px[2] = px[3] = 0;
                NFS2_ASSERT(false);
            }
        }
        /* Level L of this tile goes to atlas level L at (x >> L, y >> L).
         * The allocator only ever splits a slot into aligned quadrants,
"         * so a tile of size S is aligned to S and the shifted origin is
         * exact -- neighbouring tiles stay separate at every level.
         *
         * This loop used to "break" after the first level and the atlas
         * had a single level anyway, so the game uploaded a full mipmap
         * chain and the port kept only the top of it.  Everything was
         * therefore sampled at full resolution however small it got on
         * screen, which is invisible in a still frame and boils in
         * motion. */
        textureData += size_t(largeMipmapSize) * largeMipmapSize * 4;
        data = texelBytesData + largeMipmapSize * largeMipmapSize * texelBytes;
    }
    (*info)->mipLevels = lod ? lod : 1;
    const x86::reg32 levels = lod;
    glthread::post(m_renderer, [this, x, y, topSize, levels, converted = std::move(converted)]() {
        glBindTexture(GL_TEXTURE_2D, m_atlas);
        const x86::reg8* level = converted.data();
        x86::reg32 size = topSize;
        for (x86::reg32 lod = 0; lod < levels; ++lod, size >>= 1)
        {
            glTexSubImage2D(GL_TEXTURE_2D, GLint(lod), GLint(x >> lod), GLint(y >> lod),
                            GLsizei(size), GLsizei(size), GL_RGBA, GL_UNSIGNED_BYTE, level);
            level += size_t(size) * size * 4;
        }
    });
}

void GlideRenderer::setAlphaTestRef(x86::reg32 value)
{
    const float ref = float(value & 0xff) / 255.f;
    if (ref != m_alphaTestRef)
    {
        flush();
        m_alphaTestRef = ref;
    }
}

void GlideRenderer::setDither(bool enable)
{
    // RGBA8 output has no RGB565 quantization to hide with Bayer noise.
    NFS2_USE(enable);
    enable = false;
    if (enable != m_dither)
    {
        flush();
        m_dither = enable;
    }
}

void GlideRenderer::setCullMode(x86::reg32 mode)
{
    const bool enable = mode != 0;
    if (enable != m_cull || mode != m_cullMode)
    {
        /* Diagnostic for the rear-view mirror investigation.  Remove once
         * the mirror is confirmed fixed. */
#ifdef NFS_TRACE_MSG
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                    "[MIRROR] grCullMode %u -> %u", m_cullMode, mode);
#endif
        flush();
        m_cull = enable;
        m_cullMode = mode;
    }
}

void GlideRenderer::setTexClampMode(bool wrapS, bool wrapT)
{
    /* Written into every vertex, so no flush is needed: the value travels
     * with the geometry rather than living in the draw-call state. */
    m_texWrapS = wrapS;
    m_texWrapT = wrapT;
}

void GlideRenderer::setChromakeyMode(bool enable)
{
    if (enable != m_chromaKey)
    {
        flush();
        m_chromaKey = enable;
    }
}

void GlideRenderer::setChromakeyValue(x86::reg32 color)
{
    m_chromaKeyColor = color;
}

void GlideRenderer::setClipWindow(x86::reg32 minX, x86::reg32 minY, x86::reg32 maxX, x86::reg32 maxY)
{
    if (minX != m_clipMinX || minY != m_clipMinY || maxX != m_clipMaxX || maxY != m_clipMaxY)
    {
        /* Diagnostic for the rear-view mirror investigation: only logs on a
         * real change, so this is not per-draw-call spam. Remove once the
         * mirror is confirmed fixed. */
#ifdef NFS_TRACE_MSG
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                    "[MIRROR] grClipWindow %u,%u -> %u,%u  (was %u,%u -> %u,%u, vtx=%u)",
                    minX, minY, maxX, maxY, m_clipMinX, m_clipMinY, m_clipMaxX, m_clipMaxY, m_vertexCount);
#endif
        flush();
        m_clipMinX = minX;
        m_clipMinY = minY;
        m_clipMaxX = maxX;
        m_clipMaxY = maxY;
    }
}

void GlideRenderer::setGamma(float correction)
{
    if (correction != m_gamma)
    {
        flush();
        m_gamma = correction;
    }
}

void GlideRenderer::setFogMode(x86::reg32 mode)
{
    /* Low bits pick the source (0 disable, 1 iterated alpha, 2 table); 0x100
     * and 0x200 are MULT2/ADD2 modifiers that only scale the fog colour, which
     * nothing here needs to reproduce exactly.
     *
     * No flush() needed: drawTriangle() reads m_fogMode as it takes each
     * triangle and marks the corners fogged or not -- a vertex already in the
     * batch is unaffected by a later mode change, so there is nothing to
     * protect by ending the batch early. */
    m_fogMode = mode;
}

void GlideRenderer::setFogColor(x86::reg32 color)
{
    /* GrColor_t here is the same 0xAARRGGBB the rest of this file assumes. */
    const float r = float((color >> 16) & 0xff) / 255.f;
    const float g = float((color >> 8) & 0xff) / 255.f;
    const float b = float(color & 0xff) / 255.f;
    if (r != m_fogColor[0] || g != m_fogColor[1] || b != m_fogColor[2])
    {
        flush();
        m_fogColor[0] = r;
        m_fogColor[1] = g;
        m_fogColor[2] = b;
    }
}

void GlideRenderer::setFogTable(const x86::reg8* table)
{
    /* The vertex shader turns each corner's w into its fog factor with the
     * table of the draw call the corner is in, so a new table ends the batch
     * and starts one of its own.
     *
     * The game calls this far more often than it changes the table --
     * observed at 10000+ calls/sec on the pause menu's live 3D preview,
     * apparently once per nearby object -- and a draw-call boundary for each
     * call once pegged a CPU core and made the Camera settings screen look
     * hung.  So an exact repeat is let through untouched; what is left is a
     * real change, four a frame in a split-screen race (2026-09-25). */
    if (m_fogTableValid && memcmp(table, m_fogTableRaw, sizeof(m_fogTableRaw)) == 0)
    {
        return;
    }
    ++s_counters.fogTables;
    flush();
    memcpy(m_fogTableRaw, table, sizeof(m_fogTableRaw));
    m_fogTableValid = true;
    std::array<float, 64> factors;
    for (int i = 0; i < 64; ++i)
    {
        factors[i] = float(table[i]) / 255.f;
    }
    m_fogTables.push_back(factors);
    m_fogTableIndex = int(m_fogTables.size()) - 1;
}


/* Texture coordinates far from the origin, brought back near it.
 *
 * With projected headlights the game draws the lit road a second time over
 * itself -- its own wrapped surface texture, darkened by vertex colour, depth
 * test always passing -- and that pass reaches the Glide layer with s and t a
 * thousand to eight thousand Glide units from the origin, up to thirty repeats
 * of the texture (a night race, 2026-09-25).  Interpolated and divided per
 * pixel, Mali kept too little of the fraction of a texel at that size: the
 * bilinear weights came out in steps and the lit road turned into moving
 * blocks, as it does in the Modern Patch under dgVoodoo and not under nGlide.
 * Adreno showed nothing.  Near the origin the same pass draws clean.
 *
 * A wrapped axis repeats every 256 Glide units (every tile, in the shader), so
 * taking a whole number of those off a triangle changes nothing it samples:
 * s/w - N*256*(1/w) at each corner is S - N*256 after the divide.  Worked out
 * in double, where the difference of the game's floats is exact.  Only when a
 * corner is past four repeats; everything else keeps its coordinates to the
 * bit.  NFS_TEXCOORD_REBASE=0 turns it off, to compare. */
static bool texCoordRebase()
{
    static const bool on = []() {
        const char* value = SDL_getenv("NFS_TEXCOORD_REBASE");
        return !value || SDL_strcmp(value, "0") != 0;
    }();
    return on;
}

static const float s_rebaseLimit = 1024.f;

static bool farFromOrigin(const float coord[3], const float oow[3])
{
    return std::fabs(coord[0]) > s_rebaseLimit * oow[0] || std::fabs(coord[1]) > s_rebaseLimit * oow[1]
        || std::fabs(coord[2]) > s_rebaseLimit * oow[2];
}

static void rebaseAxis(float coord[3], const float oow[3])
{
    if (!(oow[0] > 0.f && oow[1] > 0.f && oow[2] > 0.f))
        return;
    const double lowest = std::min(std::min(double(coord[0]) / oow[0], double(coord[1]) / oow[1]),
                                   double(coord[2]) / oow[2]);
    const double shift = std::floor(lowest / 256.0) * 256.0;
    for (int i = 0; i < 3; ++i)
        coord[i] = float(double(coord[i]) - shift * double(oow[i]));
}

/* A clamped axis cannot be shifted like that: the texture does not repeat, and
 * the headlights' light pool -- clamped both ways, one mipmap level, added over
 * the road -- comes with its corners up to ten thousand Glide units out on the
 * night tracks where the texture itself spans 256 (Atlantica, 2026-09-26).  The
 * same loss of the fraction turns the pool into a trembling mosaic there.
 *
 * What clamping does leaves a way out.  With one level, the shader samples a
 * clamped axis at pos = S * tile / 256 held to [0.5, tile - 0.5] texels, so
 * wherever S is at or below 128 / tile Glide units it reads exactly the first
 * row of texels, and at or above 256 - 128 / tile exactly the last.  So the
 * triangle is cut along those two lines, which are straight on the screen (s/w
 * and 1/w both are): the piece between them keeps its coordinates, all of them
 * near the texture now, and each piece outside takes a constant S just past its
 * line, which samples what it sampled before.  Everything a triangle carries is
 * linear on the screen -- the port's projection leaves w at 1 -- so the pieces
 * together draw what the triangle drew, up to the rounding of their new
 * corners. */
struct CutCorner
{
    double x, y, z, oow, s, t, r, g, b, a;
};

static CutCorner cutBetween(const CutCorner& p, const CutCorner& q, double u)
{
    CutCorner m;
    m.x = p.x + (q.x - p.x) * u;
    m.y = p.y + (q.y - p.y) * u;
    m.z = p.z + (q.z - p.z) * u;
    m.oow = p.oow + (q.oow - p.oow) * u;
    m.s = p.s + (q.s - p.s) * u;
    m.t = p.t + (q.t - p.t) * u;
    m.r = p.r + (q.r - p.r) * u;
    m.g = p.g + (q.g - p.g) * u;
    m.b = p.b + (q.b - p.b) * u;
    m.a = p.a + (q.a - p.a) * u;
    return m;
}

/* The convex polygon `in` cut where the coordinate of `axis` (0 s, 1 t) is
 * `edge` Glide units: the part below the line and the part on or above it. */
static void cutAt(const std::vector<CutCorner>& in, int axis, double edge,
                  std::vector<CutCorner>& below, std::vector<CutCorner>& above)
{
    below.clear();
    above.clear();
    const std::size_t n = in.size();
    for (std::size_t i = 0; i < n; ++i)
    {
        const CutCorner& p = in[i];
        const CutCorner& q = in[(i + 1) % n];
        const double fp = (axis ? p.t : p.s) - edge * p.oow;
        const double fq = (axis ? q.t : q.s) - edge * q.oow;
        (fp < 0.0 ? below : above).push_back(p);
        if ((fp < 0.0) != (fq < 0.0))
        {
            const CutCorner m = cutBetween(p, q, fp / (fp - fq));
            below.push_back(m);
            above.push_back(m);
        }
    }
}

/* The pieces of a polygon along one clamped axis: below the first row of
 * texels, over the texture, past the last row; the outer two with that
 * axis's coordinate held just outside. */
static void cutClampedAxis(std::vector<std::vector<CutCorner>>& pieces, int axis, double tile)
{
    const double low = 128.0 / tile;
    const double high = 256.0 - 128.0 / tile;
    std::vector<std::vector<CutCorner>> result;
    std::vector<CutCorner> below, rest, middle, above;
    for (const std::vector<CutCorner>& piece : pieces)
    {
        cutAt(piece, axis, low, below, rest);
        cutAt(rest, axis, high, middle, above);
        for (CutCorner& corner : below)
            (axis ? corner.t : corner.s) = (low - 8.0) * corner.oow;
        for (CutCorner& corner : above)
            (axis ? corner.t : corner.s) = (high + 8.0) * corner.oow;
        for (std::vector<CutCorner>* part : { &below, &middle, &above })
            if (part->size() >= 3)
                result.push_back(*part);
    }
    pieces.swap(result);
}

/* NFS_TEXCOORD_TRACE=1: once a second, how many triangles came in with far
 * coordinates and how far, and the first few of them with the Glide state
 * they were drawn in -- to see the headlight passes for what they are. */
static bool texCoordTrace()
{
    static const bool on = []() {
        const char* value = SDL_getenv("NFS_TEXCOORD_TRACE");
        return value && SDL_strcmp(value, "1") == 0;
    }();
    return on;
}

void GlideRenderer::drawTriangle(const GrVertex* a, const GrVertex* b, const GrVertex* c)
{
    NFS2_ASSERT(65535.f - a->ooz >= 0.f);
    NFS2_ASSERT(65535.f - b->ooz >= 0.f);
    NFS2_ASSERT(65535.f - c->ooz >= 0.f);
    NFS2_ASSERT(65535.f - a->ooz <= 65535.f);
    NFS2_ASSERT(65535.f - b->ooz <= 65535.f);
    NFS2_ASSERT(65535.f - c->ooz <= 65535.f);
    /* Nothing used to check the room: the assert at the end runs after the
     * writes and is compiled out of every build that ships.  Should a frame
     * ever fill even the larger queue, draw what is queued now -- a texture
     * upload already does that mid-frame -- rather than write past it. */
    if (m_vertexCount + 3 > s_maxVertexCount)
    {
        static unsigned fullQueues = 0;
        if (++fullQueues <= 4u || (fullQueues % 512u) == 0u)
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                        "[GFX] vertex queue full at %u vertices, drawn early (case %u)",
                        unsigned(m_vertexCount), fullQueues);
        renderPending();
    }
    /* What the three corners share -- the texture's place in the atlas, how
     * it wraps and combines -- worked out once for the triangle rather than
     * once for each corner: this runs for every triangle of every frame, and
     * a profile of a split-screen race had it at a tenth of the game thread.
     * The fog factor is the vertex shader's; here only whether there is fog. */
    GlVertex shared;
    shared.atlasInfo[0] = std::uint16_t(m_atlasSize);
    shared.atlasInfo[1] = std::uint16_t(m_textureOffsetW);
    shared.atlasInfo[2] = std::uint16_t(m_textureOffsetX);
    shared.atlasInfo[3] = std::uint16_t(m_textureOffsetY);
    shared.combine[0] = x86::reg8(m_colorFactor != 0.f);
    shared.combine[1] = x86::reg8(m_alphaFactor != 0.f);
    shared.combine[2] = x86::reg8((m_texWrapS ? 1 : 0) + (m_texWrapT ? 2 : 0) + 4 * int(m_textureMipLevels));
    /* mode 2 is GR_FOG_WITH_TABLE; iterated-alpha fog (mode 1) would take the
     * factor from the vertex alpha instead, but NFS3 uses the table. */
    const bool fogged = (m_fogMode & 0x3) == 0x2;
    shared.combine[3] = x86::reg8(fogged);
    const GrVertex* const corners[3] = { a, b, c };
    float sow[3] = { a->tmuvtx[0].sow, b->tmuvtx[0].sow, c->tmuvtx[0].sow };
    float tow[3] = { a->tmuvtx[0].tow, b->tmuvtx[0].tow, c->tmuvtx[0].tow };
    const float oow[3] = { a->oow, b->oow, c->oow };
    const bool farS = farFromOrigin(sow, oow);
    const bool farT = farFromOrigin(tow, oow);
    if (farS || farT)
    {
        if (texCoordTrace())
            traceFarTriangle(corners, farS, farT);
        if (texCoordRebase())
        {
            if (farS && m_texWrapS)
                rebaseAxis(sow, oow);
            if (farT && m_texWrapT)
                rebaseAxis(tow, oow);
            // A clamped axis far out, on a texture of one level: cut instead
            // (NFS_TEXCOORD_CUT=1; off by default since the flicker in the
            // light pool turned out to be depth, 2026-09-26).
            static const bool cutting = []() {
                const char* value = SDL_getenv("NFS_TEXCOORD_CUT");
                return value && SDL_strcmp(value, "1") == 0;
            }();
            const bool cutS = cutting && farS && !m_texWrapS;
            const bool cutT = cutting && farT && !m_texWrapT;
            if ((cutS || cutT) && m_textureMipLevels == 1 && m_textureOffsetW > 0
                && oow[0] > 0.f && oow[1] > 0.f && oow[2] > 0.f)
            {
                std::vector<std::vector<CutCorner>> pieces(1);
                for (int i = 0; i < 3; ++i)
                {
                    const GrVertex* in = corners[i];
                    pieces[0].push_back({ in->x, in->y, in->ooz, in->oow, sow[i], tow[i],
                                          in->r, in->g, in->b, in->a });
                }
                const double tile = double(m_textureOffsetW);
                if (cutS)
                    cutClampedAxis(pieces, 0, tile);
                if (cutT)
                    cutClampedAxis(pieces, 1, tile);
                auto corner = [&](GlVertex* at, const CutCorner& from) {
                    at->x = float(from.x);
                    at->y = float(from.y);
                    at->z = float(from.z);
                    at->color = (x86::reg8)(float(from.r))
                              | (x86::reg8)(float(from.g)) << 8
                              | (x86::reg8)(float(from.b)) << 16
                              | (x86::reg8)(float(from.a)) << 24;
                    at->u = float(from.s);
                    at->v = float(from.t);
                    at->oow = float(from.oow);
                    std::memcpy(at->atlasInfo, shared.atlasInfo, sizeof shared.atlasInfo);
                    std::memcpy(at->combine, shared.combine, sizeof shared.combine);
                };
                for (const std::vector<CutCorner>& piece : pieces)
                {
                    for (std::size_t i = 1; i + 1 < piece.size(); ++i)
                    {
                        if (m_vertexCount + 3 > s_maxVertexCount)
                            renderPending();
                        GlVertex* at = m_vertices + m_vertexCount;
                        corner(at, piece[0]);
                        corner(at + 1, piece[i]);
                        corner(at + 2, piece[i + 1]);
                        m_vertexCount += 3;
                    }
                }
                ++s_counters.triangles;
                if (fogged)
                    ++s_counters.fogged;
                ++s_counters.cut;
                return;
            }
        }
    }
    GlVertex* out = m_vertices + m_vertexCount;
    for (int i = 0; i < 3; ++i)
    {
        const GrVertex* in = corners[i];
        out->x = in->x;
        out->y = in->y;
        out->z = in->ooz;
        out->color = (x86::reg8)(in->r)
                   | (x86::reg8)(in->g) << 8
                   | (x86::reg8)(in->b) << 16
                   | (x86::reg8)(in->a) << 24;
        out->u = sow[i];
        out->v = tow[i];
        out->oow = in->oow;
        std::memcpy(out->atlasInfo, shared.atlasInfo, sizeof shared.atlasInfo);
        std::memcpy(out->combine, shared.combine, sizeof shared.combine);
        ++out;
    }
    m_vertexCount += 3;
    ++s_counters.triangles;
    if (fogged)
        ++s_counters.fogged;

    NFS2_ASSERT(m_vertexCount <= s_maxVertexCount);
}

GlideCounters GlideRenderer::takeCounters()
{
    const GlideCounters counters = s_counters;
    s_counters = GlideCounters();
    return counters;
}

void GlideRenderer::traceFarTriangle(const GrVertex* const corners[3], bool farS, bool farT)
{
    static Uint64 since = 0;
    static unsigned count = 0, samples = 0;
    static double largestS = 0.0, largestT = 0.0;
    const Uint64 now = SDL_GetTicks();
    if (since == 0)
        since = now;
    ++count;
    for (int i = 0; i < 3; ++i)
    {
        const GrVertex* v = corners[i];
        if (v->oow > 0.f)
        {
            largestS = std::max(largestS, std::fabs(double(v->tmuvtx[0].sow) / v->oow));
            largestT = std::max(largestT, std::fabs(double(v->tmuvtx[0].tow) / v->oow));
        }
    }
    if (samples < 24 || (samples < 60 && count % 600 == 0))
    {
        ++samples;
        char corner[3][96];
        for (int i = 0; i < 3; ++i)
        {
            const GrVertex* v = corners[i];
            SDL_snprintf(corner[i], sizeof corner[i], "s/w %.6g t/w %.6g 1/w %.6g z %.1f (S %.1f T %.1f)",
                         double(v->tmuvtx[0].sow), double(v->tmuvtx[0].tow), double(v->oow), double(v->ooz),
                         v->oow > 0.f ? double(v->tmuvtx[0].sow) / v->oow : 0.0,
                         v->oow > 0.f ? double(v->tmuvtx[0].tow) / v->oow : 0.0);
        }
        SDL_Log("[TEXCOORD] far %s%s wrap %d%d tile %u at %u,%u mips %u blend %s ztest %d zwrite %d always %d "
                "fog %u colour %.0f alpha %.0f key %d rgba %.0f,%.0f,%.0f,%.0f",
                farS ? "S" : "", farT ? "T" : "", int(m_texWrapS), int(m_texWrapT), unsigned(m_textureOffsetW),
                unsigned(m_textureOffsetX), unsigned(m_textureOffsetY), unsigned(m_textureMipLevels),
                m_alphaBlend ? "1-srcalpha" : "add", int(m_zTest), int(m_zBuffer), int(m_depthAlways),
                unsigned(m_fogMode), double(m_colorFactor), double(m_alphaFactor), int(m_chromaKey),
                double(corners[0]->r), double(corners[0]->g), double(corners[0]->b), double(corners[0]->a));
        for (int i = 0; i < 3; ++i)
            SDL_Log("[TEXCOORD]   %s", corner[i]);
    }
    if (now - since >= 1000)
    {
        SDL_Log("[TEXCOORD] %u far triangles in %u ms (%s), largest |S| %.0f |T| %.0f", count, unsigned(now - since),
                texCoordRebase() ? "brought near the origin" : "left as they are", largestS, largestT);
        since = now;
        count = 0;
        largestS = largestT = 0.0;
    }
}

}
