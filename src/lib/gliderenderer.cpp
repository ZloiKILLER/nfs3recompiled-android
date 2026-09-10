#include <lib/gliderenderer.h>
#include <lib/renderer.h>
#include <lib/glidetmu.h>
#include <SDL3/SDL.h>
#include <lib/glcompat.h>
#include <lib/glfuncs.h>
#include <cstring>

namespace win32
{

static const char g_glslPreamble[] = NFS_GLSL_PREAMBLE;


static const char g_glVertexShader[] = ""
"in vec3 g_position;"
"in vec4 g_color;"
"in vec2 g_texCoord;"
"in vec4 g_combine;"
"in vec4 g_atlasInfo;"
"in float g_fog;"
"out vec3 v_texCoord;"
"out vec4 v_color;"
"out float v_fog;"
"flat out vec4 v_combine;"
"flat out vec4 v_atlasInfo;"
"uniform mat4 u_transform;"
"void main()"
"{"
"    v_texCoord = vec3(g_texCoord.st, g_combine.q);"
"    v_combine = g_combine;"
"    v_color = g_color;"
"    v_atlasInfo = g_atlasInfo;"
"    v_fog = g_fog;"
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
"    vec4 textureColor = (t00 * wgt.x + t10 * wgt.y"
"                       + t01 * wgt.z + t11 * wgt.w) / wsum;"
"    vec4 color = vec4(mix(v_color.rgb, v_color.rgb * textureColor.rgb, v_combine.s),"
"                      mix(v_color.a, v_color.a * textureColor.a, v_combine.t));"
"    if (color.a <= u_alphaRef) discard;"
/* Glide fogs the colour only; alpha is left alone. */
"    color.rgb = mix(color.rgb, u_fogColor, clamp(v_fog, 0.0, 1.0));"
"    color.rgb = pow(max(color.rgb, vec3(0.0)), vec3(1.0 / max(u_gamma, 0.001)));"
"    if (u_dither) {"
"        const float bayer[16] = float[16](0.0, 8.0, 2.0, 10.0, 12.0, 4.0, 14.0, 6.0,"
"                                           3.0, 11.0, 1.0, 9.0, 15.0, 7.0, 13.0, 5.0);"
"        int bx = int(mod(gl_FragCoord.x, 4.0));"
"        int by = int(mod(gl_FragCoord.y, 4.0));"
"        float d = (bayer[by * 4 + bx] - 7.5) / 16.0;"
"        color.rgb += d * vec3(1.0 / 31.0, 1.0 / 63.0, 1.0 / 31.0);"
"    }"
"    o_color = color;"
"}";

struct GlVertex
{
    float       x, y, z;
    x86::reg32  color;
    float       u, v;
    float       colorCombine, alphaCombine, texWrap, u2;
    float       atlasSize, width, offX, offY;
    float       fog;
};

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

static const x86::reg32 s_maxVertexCount = 32000;
static const x86::reg32 s_atlasSize = 2048;

GlideRenderer::GlideRenderer(Renderer* renderer)
    :   m_renderer(renderer)
    ,   m_vertices(new GlVertex[s_maxVertexCount])
    ,   m_vertexCount(0)
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
    for (int i = 0; i < 64; ++i)
    {
        m_fogTable[i] = 0.f;
    }
    m_tmus[0] = new GlideTMU(s_atlasSize);
    //m_tmus[1] = new GlideTMU(s_atlasSize);
    m_renderer->setCurrent();
    loadGlFunctions();

    compileShaders();
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
    glTexStorage2D(GL_TEXTURE_2D, 9, GL_RGBA8, s_atlasSize, s_atlasSize);
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
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_renderer->clearCurrent();
}

GlideRenderer::~GlideRenderer()
{
    delete[] m_vertices;
    delete m_tmus[0];
    glDeleteFramebuffers(1, &m_framebuffer);
    free(m_textureUploadScratch);
}

void GlideRenderer::compileShaders()
{
    GLuint vertexShader, fragmentShader;
    const GLint vertexShaderLen[2] = { GLint(sizeof(g_glslPreamble) - 1),
                                       GLint(sizeof(g_glVertexShader) - 1) };
    const GLint fragmentShaderLen[2] = { GLint(sizeof(g_glslPreamble) - 1),
                                         GLint(sizeof(g_glFragmentShader) - 1) };
    const GLchar* vertexShaderSrc[2] = { g_glslPreamble, g_glVertexShader };
    const GLchar* fragmentShaderSrc[2] = { g_glslPreamble, g_glFragmentShader };
    GLint status = 0;

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
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 2, fragmentShaderSrc, fragmentShaderLen);
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

    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vertexShader);
    glAttachShader(m_shaderProgram, fragmentShader);
    glLinkProgram(m_shaderProgram);

    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &status);
    if (!status)
    {
        GLint maxLen = 0;
        GLsizei len = 0;
        glGetProgramiv(m_shaderProgram, GL_INFO_LOG_LENGTH, &maxLen);
        if (maxLen > 1)
        {
            GLchar *log = (GLchar *)malloc(maxLen);
            glGetProgramInfoLog(m_shaderProgram, maxLen, &len, log);
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "%s", log);
            free(log);
        }
    }
    m_attributes[0] = glGetAttribLocation(m_shaderProgram, "g_position");
    m_attributes[1] = glGetAttribLocation(m_shaderProgram, "g_color");
    m_attributes[2] = glGetAttribLocation(m_shaderProgram, "g_texCoord");
    m_attributes[3] = glGetAttribLocation(m_shaderProgram, "g_combine");
    m_attributes[4] = glGetAttribLocation(m_shaderProgram, "g_atlasInfo");
    m_attributes[5] = glGetAttribLocation(m_shaderProgram, "g_fog");
    m_transform = glGetUniformLocation(m_shaderProgram, "u_transform");
    m_fogColorUniform = glGetUniformLocation(m_shaderProgram, "u_fogColor");
    m_alphaRefUniform = glGetUniformLocation(m_shaderProgram, "u_alphaRef");
    m_ditherUniform = glGetUniformLocation(m_shaderProgram, "u_dither");
    m_chromaKeyUniform = glGetUniformLocation(m_shaderProgram, "u_chromaKey");
    m_chromaKeyColorUniform = glGetUniformLocation(m_shaderProgram, "u_chromaKeyColor");
    m_gammaUniform = glGetUniformLocation(m_shaderProgram, "u_gamma");
    //glUniform1i(glGetUniformLocation(m_shaderProgram, "u_texture"), 0);
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
    m_renderer->setCurrent();
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
    if (m_clipMaxX > m_clipMinX && m_clipMaxY > m_clipMinY)
    {
        glEnable(GL_SCISSOR_TEST);
        glScissor(m_clipMinX, m_clipMinY,
                  m_clipMaxX - m_clipMinX, m_clipMaxY - m_clipMinY);
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
    m_renderer->clearCurrent();
}

void GlideRenderer::render(x86::reg32 buffer)
{
    m_renderer->m_currentBuffer = buffer;
}

void GlideRenderer::flush()
{
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
    m_renderer->setCurrent();
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderer->m_texture, 0);
    {
        flush();
        /* Renderer::present() binds its own vertex array for the blit, so this
         * one can no longer rely on the binding made in the constructor. */
        glBindVertexArray(m_vertexArray);
        glViewport(0, 0, m_renderer->m_width, m_renderer->m_height);
        glUseProgram(m_shaderProgram);
        /* This used to be glOrtho(0, w, 0, h, 0, -65536) read back with
         * glGetFloatv(GL_PROJECTION_MATRIX).  Neither the fixed-function matrix
         * stack nor that query exists in GLES, and the shader already takes the
         * matrix through u_transform, so build it here instead.  Column-major,
         * exactly what glOrtho(l=0, r=w, b=0, t=h, n=0, f=-65536) produced:
         * diagonal 2/(r-l), 2/(t-b), -2/(f-n) and translation -(r+l)/(r-l),
         * -(t+b)/(t-b), -(f+n)/(f-n), which collapses to -1 on every axis. */
        const float w = float(m_renderer->m_width);
        const float h = float(m_renderer->m_height);
        const float matrix[16] = {
            2.0f / w, 0.0f,     0.0f,           0.0f,
            0.0f,     2.0f / h, 0.0f,           0.0f,
            0.0f,     0.0f,     2.0f / 65536.0f, 0.0f,
            -1.0f,    -1.0f,    -1.0f,          1.0f,
        };
        glUniformMatrix4fv(m_transform, 1, GL_FALSE, matrix);
        glBindTexture(GL_TEXTURE_2D, m_atlas);
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GlVertex)*m_vertexCount, m_vertices, GL_STREAM_DRAW);
        glVertexAttribPointer(m_attributes[0], 3, GL_FLOAT, GL_FALSE, sizeof(GlVertex), 0);
        glEnableVertexAttribArray(m_attributes[0]);
        glVertexAttribPointer(m_attributes[1], 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(GlVertex), (const void*)(4*3));
        glEnableVertexAttribArray(m_attributes[1]);
        glVertexAttribPointer(m_attributes[2], 2, GL_FLOAT, GL_FALSE, sizeof(GlVertex), (const void*)(4*4));
        glEnableVertexAttribArray(m_attributes[2]);
        glVertexAttribPointer(m_attributes[3], 4, GL_FLOAT, GL_FALSE, sizeof(GlVertex), (const void*)(4*6));
        glEnableVertexAttribArray(m_attributes[3]);
        glVertexAttribPointer(m_attributes[4], 4, GL_FLOAT, GL_FALSE, sizeof(GlVertex), (const void*)(4*10));
        glEnableVertexAttribArray(m_attributes[4]);
        glVertexAttribPointer(m_attributes[5], 1, GL_FLOAT, GL_FALSE, sizeof(GlVertex), (const void*)(4*14));
        glEnableVertexAttribArray(m_attributes[5]);
        glUniform3f(m_fogColorUniform, m_fogColor[0], m_fogColor[1], m_fogColor[2]);
        glUniform1f(m_alphaRefUniform, m_alphaTestRef);
        glUniform3f(m_chromaKeyColorUniform, float(m_chromaKeyColor >> 16 & 0xff) / 255.f,
                    float(m_chromaKeyColor >> 8 & 0xff) / 255.f,
                    float(m_chromaKeyColor & 0xff) / 255.f);
        x86::reg32 first = 0;
        for (std::vector<DrawCall>::const_iterator it = m_drawCalls.begin(); it != m_drawCalls.end(); ++it)
        {
            if (it->zTest)
            {
                glEnable(GL_DEPTH_TEST);
                /* GR_CMP_ALWAYS (grDepthBufferFunction) still writes depth
                 * per grDepthMask on real hardware -- it is not the same as
                 * disabling the test.  See setDepthAlways()'s header comment;
                 * this is what makes the rear-view mirror's clipped
                 * "always passes, write fresh depth" background pass work
                 * instead of leaving the main scene's depth in place. */
                glDepthFunc(it->zTestAlways ? GL_ALWAYS : GL_LEQUAL);
            }
            else
            {
                glDisable(GL_DEPTH_TEST);
            }
            glDepthMask(it->zBuffer ? GL_TRUE : GL_FALSE);
            if (it->alphaBlend)
            {
                glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
            }
            else
            {
                glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
            }
            glUniform1i(m_ditherUniform, it->dither ? 1 : 0);
            glUniform1i(m_chromaKeyUniform, it->chromaKey ? 1 : 0);
            glUniform1f(m_gammaUniform, it->gamma);
            if (it->cull)
            {
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
                glFrontFace(it->cullMode == 2 ? GL_CW : GL_CCW);
            }
            else
            {
                glDisable(GL_CULL_FACE);
            }
            if (it->clipMaxX > it->clipMinX && it->clipMaxY > it->clipMinY)
            {
                glEnable(GL_SCISSOR_TEST);
                /* No Y flip.  The Glide projection maps guest y straight to
                 * GL y (y_ndc = 2*y/h - 1), so a guest rectangle at y 7..110
                 * really does occupy GL rows 7..110 -- the flip to screen
                 * orientation happens later, in the blit quad in
                 * Renderer::present().  Flipping the scissor too put it at
                 * rows 658..761, the opposite edge of the buffer, not
                 * overlapping the geometry at all, so everything drawn under
                 * a narrow clip window was cut away.  Harmless for the
                 * full-screen clip the game uses everywhere else, which is
                 * why only the rear-view mirror was affected. */
                glScissor(it->clipMinX, it->clipMinY,
                          it->clipMaxX - it->clipMinX, it->clipMaxY - it->clipMinY);
            }
            else
            {
                glDisable(GL_SCISSOR_TEST);
            }
            glDrawArrays(GL_TRIANGLES, first, it->lastTriangle-first);
            first = it->lastTriangle;
        }
        m_drawCalls.clear();
    }
    m_vertexCount = 0;
    ++m_renderStamp;
}

void GlideRenderer::swap()
{
    m_renderer->setCurrent();
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderer->m_texture, 0);
    /* This used to query the display mode and set the swap interval to
     * refresh/60 on every single frame -- two EGL round trips per frame, and it
     * silently overrode the 30 Hz pacing Renderer negotiated.  One shared
     * negotiation, cached after the first call. */
    m_renderer->ensureFramePacing();
    renderPending();
    /* The presentation clear and blit must not inherit the last Glide clip. */
    glDisable(GL_SCISSOR_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
    /* No glFlush() here: this is still the same GL context/thread as the
     * present() call right below, and GL commands within one context already
     * execute in submission order without one -- the earlier flush() only
     * ever duplicated the one present() already does before SDL_GL_SwapWindow
     * (see renderer.cpp), which is the one that actually matters for
     * frame-pacing/timing. */
    glBindTexture(GL_TEXTURE_2D, m_renderer->m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_renderer->present();
    m_renderer->clearCurrent();
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

    m_renderer->setCurrent();
    glBindTexture(GL_TEXTURE_2D, m_atlas);
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

    x86::reg32 x = (*info)->x;
    x86::reg32 y = (*info)->y;
    /* Pixels are written as four bytes in R,G,B,A memory order and uploaded as
     * GL_UNSIGNED_BYTE.  The previous code packed a uint32 as 0xRRGGBBAA and
     * relied on GL_UNSIGNED_INT_8_8_8_8, which does not exist in GLES and is
     * endian-sensitive; byte writes are exact on both.
     *
     * m_textureUploadScratch is sized to the largest possible mipmap
     * (256x256x4) once in the constructor -- this used to malloc() a fresh
     * buffer here and free() it below, on every single texture upload. */
    NFS2_ASSERT(largeMipmapSize <= 256);
    x86::reg8* textureData = m_textureUploadScratch;
    x86::reg32 lod = 0;
    for (; largeMipmapSize >= smallMipmapSize; ++lod, largeMipmapSize >>= 1)
    {
        const x86::reg16* pixelData = reinterpret_cast<const x86::reg16*>(data);
        for (x86::reg32 i = 0; i < largeMipmapSize*largeMipmapSize; ++i)
        {
            x86::reg8* px = textureData + i*4;
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
        glTexSubImage2D(GL_TEXTURE_2D, GLint(lod), GLint(x >> lod), GLint(y >> lod),
                        GLsizei(largeMipmapSize), GLsizei(largeMipmapSize),
                        GL_RGBA, GL_UNSIGNED_BYTE, textureData);
        data = pixelData + largeMipmapSize * largeMipmapSize;
    }
    (*info)->mipLevels = lod ? lod : 1;
    m_renderer->clearCurrent();
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
     * No flush() needed: fogFactor() below reads m_fogMode at the moment
     * drawTriangle() bakes a fog factor into each vertex, the same as
     * m_fogTable in setFogTable() -- a vertex already in the batch is
     * unaffected by a later mode change, so there is nothing to protect by
     * ending the batch early. */
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
    /* Unlike fog colour (a uniform applied to the whole draw call at flush
     * time) or fog mode/blend/z-state (also per-draw-call GL state), the fog
     * FACTOR for a vertex is computed here on the CPU, from m_fogTable, at the
     * moment drawTriangle() adds that vertex -- see fogFactor() below.  A
     * vertex already sitting in the batch keeps whatever factor it was given;
     * changing m_fogTable cannot retroactively affect it.  So there is nothing
     * for a flush() to protect here, unlike setFogMode/setFogColor, which this
     * used to be copied from without checking that.
     *
     * That copied flush() was expensive: the game calls this far more often
     * than once per frame -- observed at 10000+ calls/sec on the pause menu's
     * live 3D preview, apparently once per nearby object -- and each flush()
     * is a full draw-call boundary.  That alone was enough to peg a CPU core
     * and make the Camera settings screen look hung.  The memcmp below is
     * just to skip the float conversion loop on an exact repeat; it is an
     * optimisation, not what fixes the hang. */
    if (m_fogTableValid && memcmp(table, m_fogTableRaw, sizeof(m_fogTableRaw)) == 0)
    {
        return;
    }
    memcpy(m_fogTableRaw, table, sizeof(m_fogTableRaw));
    m_fogTableValid = true;
    for (int i = 0; i < 64; ++i)
    {
        m_fogTable[i] = float(table[i]) / 255.f;
    }
}

/* Packed for the one float slot the vertex has: bit 0 wraps S, bit 1
 * wraps T.  The shader unpacks it back into a vec2 mask. */
float GlideRenderer::texWrapAttribute() const
{
    /* One float carries three things the shader needs per vertex: bit 0
     * wraps S, bit 1 wraps T, and the rest is how many mipmap levels this
     * texture has, so the shader knows how far it may go down the chain. */
    return float((m_texWrapS ? 1 : 0) + (m_texWrapT ? 2 : 0)
               + 4 * int(m_textureMipLevels));
}

float GlideRenderer::fogFactor(float oow) const
{
    /* mode 2 is GR_FOG_WITH_TABLE; iterated-alpha fog (mode 1) would take the
     * factor from the vertex alpha instead, but NFS3 uses the table. */
    if ((m_fogMode & 0x3) != 0x2 || oow <= 0.f)
    {
        return 0.f;
    }
    const float w = 1.f / oow;
    if (w <= s_fogW[0])
    {
        return m_fogTable[0];
    }
    if (w >= s_fogW[63])
    {
        return m_fogTable[63];
    }
    int lo = 0;
    int hi = 63;
    while (hi - lo > 1)
    {
        const int mid = (lo + hi) / 2;
        if (s_fogW[mid] <= w)
            lo = mid;
        else
            hi = mid;
    }
    const float span = s_fogW[hi] - s_fogW[lo];
    const float t = span > 0.f ? (w - s_fogW[lo]) / span : 0.f;
    return m_fogTable[lo] + (m_fogTable[hi] - m_fogTable[lo]) * t;
}

void GlideRenderer::drawTriangle(const GrVertex* a, const GrVertex* b, const GrVertex* c)
{
    NFS2_ASSERT(65535.f - a->ooz >= 0.f);
    NFS2_ASSERT(65535.f - b->ooz >= 0.f);
    NFS2_ASSERT(65535.f - c->ooz >= 0.f);
    NFS2_ASSERT(65535.f - a->ooz <= 65535.f);
    NFS2_ASSERT(65535.f - b->ooz <= 65535.f);
    NFS2_ASSERT(65535.f - c->ooz <= 65535.f);
    m_vertices[m_vertexCount].x = a->x;
    m_vertices[m_vertexCount].y = a->y;
    m_vertices[m_vertexCount].z = a->ooz;
    m_vertices[m_vertexCount].color = (x86::reg8)(a->r)
                                    | (x86::reg8)(a->g) << 8
                                    | (x86::reg8)(a->b) << 16
                                    | (x86::reg8)(a->a) << 24;
    m_vertices[m_vertexCount].u = a->tmuvtx[0].sow;
    m_vertices[m_vertexCount].v = a->tmuvtx[0].tow;
    m_vertices[m_vertexCount].colorCombine = m_colorFactor;
    m_vertices[m_vertexCount].alphaCombine = m_alphaFactor;
    m_vertices[m_vertexCount].atlasSize = s_atlasSize;
    m_vertices[m_vertexCount].texWrap = texWrapAttribute();
    m_vertices[m_vertexCount].u2 = a->oow;
    m_vertices[m_vertexCount].width = float(m_textureOffsetW);
    m_vertices[m_vertexCount].offX = float(m_textureOffsetX);
    m_vertices[m_vertexCount].offY = float(m_textureOffsetY);
    m_vertices[m_vertexCount].fog = fogFactor(a->oow);
    m_vertexCount++;
    
    m_vertices[m_vertexCount].x = b->x;
    m_vertices[m_vertexCount].y = b->y;
    m_vertices[m_vertexCount].z = b->ooz;
    m_vertices[m_vertexCount].color = (x86::reg8)(b->r)
                                              | (x86::reg8)(b->g) << 8
                                              | (x86::reg8)(b->b) << 16
                                              | (x86::reg8)(b->a) << 24;
    m_vertices[m_vertexCount].u = b->tmuvtx[0].sow;
    m_vertices[m_vertexCount].v = b->tmuvtx[0].tow;
    m_vertices[m_vertexCount].colorCombine = m_colorFactor;
    m_vertices[m_vertexCount].alphaCombine = m_alphaFactor;
    m_vertices[m_vertexCount].atlasSize = s_atlasSize;
    m_vertices[m_vertexCount].texWrap = texWrapAttribute();
    m_vertices[m_vertexCount].u2 = b->oow;
    m_vertices[m_vertexCount].width = float(m_textureOffsetW);
    m_vertices[m_vertexCount].offX = float(m_textureOffsetX);
    m_vertices[m_vertexCount].offY = float(m_textureOffsetY);
    m_vertices[m_vertexCount].fog = fogFactor(b->oow);
    m_vertexCount++;

    m_vertices[m_vertexCount].x = c->x;
    m_vertices[m_vertexCount].y = c->y;
    m_vertices[m_vertexCount].z = c->ooz;
    m_vertices[m_vertexCount].color = (x86::reg8)(c->r)
                                              | (x86::reg8)(c->g) << 8
                                              | (x86::reg8)(c->b) << 16
                                              | (x86::reg8)(c->a) << 24;
    m_vertices[m_vertexCount].u = c->tmuvtx[0].sow;
    m_vertices[m_vertexCount].v = c->tmuvtx[0].tow;
    m_vertices[m_vertexCount].colorCombine = m_colorFactor;
    m_vertices[m_vertexCount].alphaCombine = m_alphaFactor;
    m_vertices[m_vertexCount].texWrap = texWrapAttribute();
    m_vertices[m_vertexCount].u2 = c->oow;
    m_vertices[m_vertexCount].atlasSize = s_atlasSize;
    m_vertices[m_vertexCount].width = float(m_textureOffsetW);
    m_vertices[m_vertexCount].offX = float(m_textureOffsetX);
    m_vertices[m_vertexCount].offY = float(m_textureOffsetY);
    m_vertices[m_vertexCount].fog = fogFactor(c->oow);
    m_vertexCount++;

    NFS2_ASSERT(m_vertexCount < s_maxVertexCount);
}

}
