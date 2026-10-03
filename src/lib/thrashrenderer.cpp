#include <lib/thrashrenderer.h>
#include <lib/renderer.h>
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

namespace win32
{

static const char g_preamble[] = NFS_GLSL_PREAMBLE;

/* GlideRenderer's vertex shader without the atlas: the fog factor from the
 * draw's table at the corner's w, worked out here (see gliderenderer.cpp). */
static const char g_vertexShader[] = ""
"in vec3 g_position;"
"in vec4 g_color;"
"in vec3 g_texCoord;"
"in vec4 g_combine;"
"out vec3 v_texCoord;"
"out vec4 v_color;"
"out float v_fog;"
"flat out vec4 v_combine;"
"uniform mat4 u_transform;"
"uniform float u_fogW[64];"
"uniform float u_fogTable[64];"
"float fogFactor(float oow)"
"{"
"    if (oow <= 0.0) return 0.0;"
"    float w = 1.0 / oow;"
"    if (w <= u_fogW[0]) return u_fogTable[0];"
"    if (w >= u_fogW[63]) return u_fogTable[63];"
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
"    v_fog = g_combine.q > 0.5 ? fogFactor(g_texCoord.p) : 0.0;"
"    gl_Position = u_transform * vec4(g_position, 1.0);"
"}";

/* The texture through the GPU's own sampler: bilinear, the nearest mipmap
 * level, wrapped or clamped -- what the game asks the Voodoo2 for, in one
 * fetch.  A Glide coordinate spans 256 over the whole texture, whatever its
 * size.
 *
 * With a chroma key the texels of the key colour have to be dropped before
 * filtering, as the Voodoo2 did, or a halo of half-keyed colour rings every
 * cut-out edge; for those draws the four texels are fetched and weighed here,
 * as GlideRenderer does for everything. */
static const char g_fragmentShader[] =
"in vec3 v_texCoord;"
"in vec4 v_color;"
"flat in vec4 v_combine;"
"in float v_fog;"
"layout (location=0) out vec4 o_color;"
"uniform sampler2D u_texture;"
"uniform vec3 u_fogColor;"
"uniform float u_alphaRef;"
"uniform bool u_chromaKey;"
"uniform ivec3 u_chromaKeyBytes;"
"uniform int u_format;"
"uniform float u_gamma;"
"uniform vec2 u_wrap;"
"uniform float u_levels;"
/* Whether a texel is the key colour, compared as GlideRenderer compares it:
 * against the bytes its own conversion to RGBA8 gave the texel -- 565 widened
 * by a shift, 1555 scaled, 4444 and 8888 exact -- so a key matches the texels
 * it matched before the textures stayed in their own formats. */
"bool keyed(vec4 t)"
"{"
"    ivec3 bytes;"
"    if (u_format == 1)"
"        bytes = ivec3(int(round(t.r * 31.0)) << 3, int(round(t.g * 63.0)) << 2, int(round(t.b * 31.0)) << 3);"
"    else if (u_format == 2)"
"        bytes = ivec3(round(t.rgb * 31.0)) * 255 / 31;"
"    else"
"        bytes = ivec3(round(t.rgb * 255.0));"
"    return all(equal(bytes, u_chromaKeyBytes));"
"}"
"vec4 texel(vec2 at, float size, int level)"
"{"
"    vec2 t = mix(clamp(at, 0.0, size - 1.0), mod(at, size), u_wrap);"
"    return texelFetch(u_texture, ivec2(t), level);"
"}"
"void main()"
"{"
"    vec2 uv = (v_texCoord.st / v_texCoord.p) / 256.0;"
"    vec4 textureColor;"
"    if (u_chromaKey) {"
"        float size = float(textureSize(u_texture, 0).x);"
"        vec2 texelPos = uv * size;"
"        float rho = max(length(dFdx(texelPos)), length(dFdy(texelPos)));"
"        float lod = clamp(floor(log2(max(rho, 1e-6)) + 0.5), 0.0, max(u_levels - 1.0, 0.0));"
"        float scale = exp2(lod);"
"        float tile = max(size / scale, 1.0);"
"        vec2 pos = texelPos / scale;"
"        pos = mix(pos, mod(pos, tile), u_wrap);"
"        pos = mix(clamp(pos, 0.5, max(tile - 0.5, 0.5)), pos, u_wrap);"
"        vec2 base = floor(pos - 0.5);"
"        vec2 w = pos - 0.5 - base;"
"        int level = int(lod);"
"        vec4 t00 = texel(base, tile, level);"
"        vec4 t10 = texel(base + vec2(1.0, 0.0), tile, level);"
"        vec4 t01 = texel(base + vec2(0.0, 1.0), tile, level);"
"        vec4 t11 = texel(base + vec2(1.0, 1.0), tile, level);"
"        vec4 keep = vec4(keyed(t00) ? 0.0 : 1.0, keyed(t10) ? 0.0 : 1.0,"
"                         keyed(t01) ? 0.0 : 1.0, keyed(t11) ? 0.0 : 1.0);"
"        vec4 wgt = vec4((1.0 - w.x) * (1.0 - w.y), w.x * (1.0 - w.y),"
"                        (1.0 - w.x) * w.y,         w.x * w.y) * keep;"
"        float wsum = wgt.x + wgt.y + wgt.z + wgt.w;"
"        if (wsum <= 0.0) discard;"
"        textureColor = (t00 * wgt.x + t10 * wgt.y + t01 * wgt.z + t11 * wgt.w) / wsum;"
"    } else {"
"        textureColor = texture(u_texture, uv);"
"    }"
"    vec4 color = vec4(mix(v_color.rgb, v_color.rgb * textureColor.rgb, v_combine.s),"
"                      mix(v_color.a, v_color.a * textureColor.a, v_combine.t));"
"    if (color.a <= u_alphaRef) discard;"
"    color.rgb = mix(color.rgb, u_fogColor, clamp(v_fog, 0.0, 1.0));"
"    if (u_gamma != 1.0)"
"        color.rgb = pow(max(color.rgb, vec3(0.0)), vec3(1.0 / max(u_gamma, 0.001)));"
"    o_color = color;"
/* The Voodoo's 16-bit depth steps where the buffer has finer ones
 * (needsDepthSteps, gliderenderer.cpp). */
"\n#ifdef NFS_DEPTH16\n"
"    gl_FragDepth = (floor(gl_FragCoord.z * 65536.0) + 0.5) / 65536.0;"
"\n#endif\n"
"}";

static const x86::reg32 s_maxVertices = ThrashRenderer::s_maxVertices;

static std::mutex s_poolMutex;
static std::vector<ThrashVertex*> s_pool;

static ThrashVertex* takeVertices()
{
    {
        std::lock_guard<std::mutex> lock(s_poolMutex);
        if (!s_pool.empty())
        {
            ThrashVertex* vertices = s_pool.back();
            s_pool.pop_back();
            return vertices;
        }
    }
    return new ThrashVertex[s_maxVertices];
}

static void giveVertices(ThrashVertex* vertices)
{
    std::lock_guard<std::mutex> lock(s_poolMutex);
    s_pool.push_back(vertices);
}

/* What renderPending() hands the GL thread. */
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

/* Glide's fog table w(i), as gliderenderer.cpp has it. */
static void fogW(float (&w)[64])
{
    for (int i = 0; i < 64; ++i)
        w[i] = float(std::pow(2.0, 3.0 + double(i >> 2)) / double(8 - (i & 3)));
}

ThrashRenderer::ThrashRenderer(Renderer* renderer)
    :   m_renderer(renderer)
    ,   m_depthBuffer(0)
    ,   m_framebuffer(0)
    ,   m_vertexBuffer(0)
    ,   m_vertexArray(0)
    ,   m_program(0)
    ,   m_samplers{0, 0, 0, 0}
    ,   m_locations{}
    ,   m_depth16(false)
    ,   m_vertices(takeVertices())
    ,   m_vertexCount(0)
    ,   m_nextTexture(0)
    ,   m_texture(0)
    ,   m_textureLevels(1)
    ,   m_textureFormat(0)
    ,   m_renderStamp(1)
    ,   m_fogTableIndex(0)
    ,   m_fogTableValid(false)
    ,   m_fogMode(0)
    ,   m_alphaTestRef(0.f)
    ,   m_colorFactor(1.f)
    ,   m_alphaFactor(1.f)
    ,   m_zBuffer(true)
    ,   m_zTest(true)
    ,   m_depthAlways(false)
    ,   m_alphaBlend(true)
    ,   m_cull(false)
    ,   m_chromaKey(false)
    ,   m_wrapS(true)
    ,   m_wrapT(true)
    ,   m_cullMode(0)
    ,   m_chromaKeyColor(0)
    ,   m_clipMinX(0)
    ,   m_clipMinY(0)
    ,   m_clipMaxX(0)
    ,   m_clipMaxY(0)
    ,   m_gamma(1.f)
{
    m_fogColor[0] = m_fogColor[1] = m_fogColor[2] = 0.f;
    m_fogTables.assign(1, std::array<float, 64>());
    glthread::post(m_renderer, [this]() { initGl(); });
    glthread::finish();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[GFX] native THRASH renderer: a texture of OpenGL's for each of the game's");
}

ThrashRenderer::~ThrashRenderer()
{
    const unsigned int framebuffer = m_framebuffer, depthBuffer = m_depthBuffer, program = m_program;
    const unsigned int vertexBuffer = m_vertexBuffer, vertexArray = m_vertexArray;
    glthread::post(m_renderer, [this, framebuffer, depthBuffer, program, vertexBuffer, vertexArray]() {
        for (unsigned int& texture : m_glTextures)
            if (texture)
                glDeleteTextures(1, &texture);
        m_glTextures.clear();
        glDeleteSamplers(4, m_samplers);
        glDeleteFramebuffers(1, &framebuffer);
        glDeleteRenderbuffers(1, &depthBuffer);
        glDeleteProgram(program);
        glDeleteBuffers(1, &vertexBuffer);
        glDeleteVertexArrays(1, &vertexArray);
    });
    glthread::finish();
    giveVertices(m_vertices);
}

void ThrashRenderer::initGl()
{
    loadGlFunctions();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glGenVertexArrays(1, &m_vertexArray);
    glBindVertexArray(m_vertexArray);
    glGenBuffers(1, &m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glGenSamplers(4, m_samplers);
    for (int i = 0; i < 4; ++i)
    {
        glSamplerParameteri(m_samplers[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glSamplerParameteri(m_samplers[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(m_samplers[i], GL_TEXTURE_WRAP_S, (i & 1) ? GL_REPEAT : GL_CLAMP_TO_EDGE);
        glSamplerParameteri(m_samplers[i], GL_TEXTURE_WRAP_T, (i & 2) ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    }

    glGenRenderbuffers(1, &m_depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_renderer->m_width, m_renderer->m_height);
    glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderer->m_texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBuffer);
    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);
    GLint depthBits = 0;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                          GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depthBits);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    compileShaders(depthBits, (const char*)glGetString(GL_RENDERER));
}

static GLuint compileShader(GLenum type, const std::string& defines, const char* body)
{
    const GLchar* sources[3] = { g_preamble, defines.c_str(), body };
    const GLuint shader = glCreateShader(type);
    glShaderSource(shader, 3, sources, nullptr);
    glCompileShader(shader);
    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        GLchar log[2048];
        GLsizei length = 0;
        glGetShaderInfoLog(shader, sizeof log, &length, log);
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "[GFX] native THRASH shader: %s", log);
    }
    return shader;
}

void ThrashRenderer::compileShaders(int depthBits, const char* renderer)
{
    m_depth16 = needsDepthSteps(depthBits, renderer);
    const char* depth16 = SDL_getenv("NFS_DEPTH16");
    if (depth16 && SDL_strcmp(depth16, "0") == 0)
        m_depth16 = false;
    else if (depth16 && SDL_strcmp(depth16, "1") == 0)
        m_depth16 = true;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[GFX] depth in %s (%d bits, %s)",
                m_depth16 ? "the Voodoo's 16-bit steps" : "the GPU's own precision", depthBits,
                renderer ? renderer : "?");

    const GLuint vertex = compileShader(GL_VERTEX_SHADER, "", g_vertexShader);
    const GLuint fragment = compileShader(GL_FRAGMENT_SHADER, m_depth16 ? "#define NFS_DEPTH16 1\n" : "",
                                          g_fragmentShader);
    m_program = glCreateProgram();
    glAttachShader(m_program, vertex);
    glAttachShader(m_program, fragment);
    glLinkProgram(m_program);
    GLint status = 0;
    glGetProgramiv(m_program, GL_LINK_STATUS, &status);
    if (!status)
    {
        GLchar log[2048];
        GLsizei length = 0;
        glGetProgramInfoLog(m_program, sizeof log, &length, log);
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "[GFX] native THRASH program: %s", log);
    }
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    m_locations.position = glGetAttribLocation(m_program, "g_position");
    m_locations.color = glGetAttribLocation(m_program, "g_color");
    m_locations.texCoord = glGetAttribLocation(m_program, "g_texCoord");
    m_locations.combine = glGetAttribLocation(m_program, "g_combine");
    m_locations.transform = glGetUniformLocation(m_program, "u_transform");
    m_locations.fogW = glGetUniformLocation(m_program, "u_fogW");
    m_locations.fogTable = glGetUniformLocation(m_program, "u_fogTable");
    m_locations.fogColor = glGetUniformLocation(m_program, "u_fogColor");
    m_locations.alphaRef = glGetUniformLocation(m_program, "u_alphaRef");
    m_locations.chromaKey = glGetUniformLocation(m_program, "u_chromaKey");
    m_locations.chromaKeyColor = glGetUniformLocation(m_program, "u_chromaKeyBytes");
    m_locations.format = glGetUniformLocation(m_program, "u_format");
    m_locations.gamma = glGetUniformLocation(m_program, "u_gamma");
    m_locations.wrap = glGetUniformLocation(m_program, "u_wrap");
    m_locations.levels = glGetUniformLocation(m_program, "u_levels");
    m_locations.texture = glGetUniformLocation(m_program, "u_texture");
    glUseProgram(m_program);
    glUniform1i(m_locations.texture, 0);
    float w[64];
    fogW(w);
    glUniform1fv(m_locations.fogW, 64, w);
    glUseProgram(0);
}

void ThrashRenderer::clear(x86::reg32 color)
{
    const x86::reg32 clipMinX = m_clipMinX, clipMinY = m_clipMinY, clipMaxX = m_clipMaxX, clipMaxY = m_clipMaxY;
    glthread::post(m_renderer, [this, color, clipMinX, clipMinY, clipMaxX, clipMaxY]() {
        clearFrame(color, clipMinX, clipMinY, clipMaxX, clipMaxY);
    });
}

/* Only the clip window is cleared, as on the Voodoo: the rear-view mirror is
 * drawn by clearing its rectangle and drawing the view behind into it. */
void ThrashRenderer::clearFrame(x86::reg32 color, x86::reg32 clipMinX, x86::reg32 clipMinY,
                                x86::reg32 clipMaxX, x86::reg32 clipMaxY)
{
    glDepthMask(true);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderer->m_texture, 0);
    if (clipMaxX > clipMinX && clipMaxY > clipMinY)
    {
        glEnable(GL_SCISSOR_TEST);
        glScissor(clipMinX, clipMinY, clipMaxX - clipMinX, clipMaxY - clipMinY);
    }
    else
    {
        glDisable(GL_SCISSOR_TEST);
    }
    glClearColor(float(color >> 16 & 0xff) / 255.0f, float(color >> 8 & 0xff) / 255.0f,
                 float(color & 0xff) / 255.0f, 0.0f);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ThrashRenderer::render(x86::reg32 buffer)
{
    m_renderer->m_currentBuffer = buffer;
}

void ThrashRenderer::flush()
{
    GlideCounters& counters = GlideRenderer::counters();
    if (m_vertexCount == (m_batches.empty() ? 0 : m_batches.back().end))
    {
        ++counters.emptyCalls;
        return;
    }
    ++counters.drawCalls;
    Batch batch;
    batch.end = m_vertexCount;
    batch.texture = m_texture;
    batch.levels = m_textureLevels;
    batch.format = m_textureFormat;
    batch.wrapS = m_wrapS;
    batch.wrapT = m_wrapT;
    batch.zTest = m_zTest;
    batch.zTestAlways = m_depthAlways;
    batch.zBuffer = m_zBuffer;
    batch.alphaBlend = m_alphaBlend;
    batch.cull = m_cull;
    batch.chromaKey = m_chromaKey;
    batch.cullMode = m_cullMode;
    batch.clipMinX = m_clipMinX;
    batch.clipMinY = m_clipMinY;
    batch.clipMaxX = m_clipMaxX;
    batch.clipMaxY = m_clipMaxY;
    batch.gamma = m_gamma;
    batch.fogTable = m_fogTableIndex;
    m_batches.push_back(batch);
}

void ThrashRenderer::renderPending()
{
    if (!m_vertexCount)
        return;
    flush();
    std::shared_ptr<ThrashDraw> draw = std::make_shared<ThrashDraw>();
    draw->vertices = m_vertices;
    draw->vertexCount = m_vertexCount;
    draw->batches.swap(m_batches);
    draw->fogTables = m_fogTables;
    std::copy(m_fogColor, m_fogColor + 3, draw->fogColor);
    draw->alphaTestRef = m_alphaTestRef;
    draw->chromaKeyColor = m_chromaKeyColor;
    draw->width = m_renderer->m_width;
    draw->height = m_renderer->m_height;
    m_vertices = takeVertices();
    glthread::post(m_renderer, [this, draw]() {
        drawPending(*draw);
        giveVertices(draw->vertices);
    });
    m_vertexCount = 0;
    m_fogTables.front() = m_fogTables[m_fogTableIndex];
    m_fogTables.resize(1);
    m_fogTableIndex = 0;
    ++m_renderStamp;
}

void ThrashRenderer::drawPending(const ThrashDraw& draw)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderer->m_texture, 0);
    glBindVertexArray(m_vertexArray);
    glViewport(0, 0, draw.width, draw.height);
    glUseProgram(m_program);
    // glOrtho(0, w, 0, h, 0, -65536), as GlideRenderer builds it.
    const float w = float(draw.width), h = float(draw.height);
    const float matrix[16] = {
        2.0f / w, 0.0f,     0.0f,            0.0f,
        0.0f,     2.0f / h, 0.0f,            0.0f,
        0.0f,     0.0f,     2.0f / 65536.0f, 0.0f,
        -1.0f,    -1.0f,    -1.0f,           1.0f,
    };
    glUniformMatrix4fv(m_locations.transform, 1, GL_FALSE, matrix);
    glActiveTexture(GL_TEXTURE0);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ThrashVertex) * draw.vertexCount, draw.vertices, GL_STREAM_DRAW);
    glVertexAttribPointer(m_locations.position, 3, GL_FLOAT, GL_FALSE, sizeof(ThrashVertex),
                          (const void*)offsetof(ThrashVertex, x));
    glEnableVertexAttribArray(m_locations.position);
    glVertexAttribPointer(m_locations.color, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ThrashVertex),
                          (const void*)offsetof(ThrashVertex, color));
    glEnableVertexAttribArray(m_locations.color);
    glVertexAttribPointer(m_locations.texCoord, 3, GL_FLOAT, GL_FALSE, sizeof(ThrashVertex),
                          (const void*)offsetof(ThrashVertex, u));
    glEnableVertexAttribArray(m_locations.texCoord);
    glVertexAttribPointer(m_locations.combine, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(ThrashVertex),
                          (const void*)offsetof(ThrashVertex, combine));
    glEnableVertexAttribArray(m_locations.combine);
    glUniform3f(m_locations.fogColor, draw.fogColor[0], draw.fogColor[1], draw.fogColor[2]);
    glUniform1f(m_locations.alphaRef, draw.alphaTestRef);
    glUniform3i(m_locations.chromaKeyColor, GLint(draw.chromaKeyColor >> 16 & 0xff),
                GLint(draw.chromaKeyColor >> 8 & 0xff), GLint(draw.chromaKeyColor & 0xff));

    x86::reg32 first = 0;
    int fogTable = -1;
    const Batch* previous = nullptr;
    bool scissor = false;
    for (const Batch& batch : draw.batches)
    {
        if (batch.fogTable != fogTable)
        {
            fogTable = batch.fogTable;
            glUniform1fv(m_locations.fogTable, 64, draw.fogTables[fogTable].data());
        }
        if (!previous || batch.texture != previous->texture)
        {
            glBindTexture(GL_TEXTURE_2D, batch.texture < m_glTextures.size() ? m_glTextures[batch.texture] : 0);
            glUniform1f(m_locations.levels, float(batch.levels));
            glUniform1i(m_locations.format, batch.format);
        }
        if (!previous || batch.wrapS != previous->wrapS || batch.wrapT != previous->wrapT)
        {
            glBindSampler(0, m_samplers[(batch.wrapS ? 1 : 0) + (batch.wrapT ? 2 : 0)]);
            glUniform2f(m_locations.wrap, batch.wrapS ? 1.f : 0.f, batch.wrapT ? 1.f : 0.f);
        }
        if (!previous || batch.zTest != previous->zTest)
        {
            if (batch.zTest)
                glEnable(GL_DEPTH_TEST);
            else
                glDisable(GL_DEPTH_TEST);
        }
        // GR_CMP_ALWAYS keeps the test, and the write, on (the rear-view mirror).
        if (batch.zTest && (!previous || !previous->zTest || batch.zTestAlways != previous->zTestAlways))
            glDepthFunc(batch.zTestAlways ? GL_ALWAYS : GL_LEQUAL);
        if (!previous || batch.zBuffer != previous->zBuffer)
            glDepthMask(batch.zBuffer ? GL_TRUE : GL_FALSE);
        if (!previous || batch.alphaBlend != previous->alphaBlend)
        {
            if (batch.alphaBlend)
                glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
            else
                glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
        }
        if (!previous || batch.chromaKey != previous->chromaKey)
            glUniform1i(m_locations.chromaKey, batch.chromaKey ? 1 : 0);
        if (!previous || batch.gamma != previous->gamma)
            glUniform1f(m_locations.gamma, batch.gamma);
        if (!previous || batch.cull != previous->cull)
        {
            if (batch.cull)
            {
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
            }
            else
            {
                glDisable(GL_CULL_FACE);
            }
        }
        if (batch.cull && (!previous || !previous->cull || batch.cullMode != previous->cullMode))
            glFrontFace(batch.cullMode == 2 ? GL_CW : GL_CCW);
        // No Y flip: the game's y is GL's here; the blit turns the picture over.
        const bool clipped = batch.clipMaxX > batch.clipMinX && batch.clipMaxY > batch.clipMinY;
        if (!previous || clipped != scissor)
        {
            if (clipped)
                glEnable(GL_SCISSOR_TEST);
            else
                glDisable(GL_SCISSOR_TEST);
        }
        if (clipped && (!previous || !scissor || batch.clipMinX != previous->clipMinX
                        || batch.clipMinY != previous->clipMinY || batch.clipMaxX != previous->clipMaxX
                        || batch.clipMaxY != previous->clipMaxY))
            glScissor(batch.clipMinX, batch.clipMinY, batch.clipMaxX - batch.clipMinX,
                      batch.clipMaxY - batch.clipMinY);
        scissor = clipped;
        previous = &batch;
        glDrawArrays(GL_TRIANGLES, GLint(first), GLsizei(batch.end - first));
        first = batch.end;
    }
    glBindSampler(0, 0);
}

void ThrashRenderer::swap()
{
    m_renderer->ensureFramePacing();
    renderPending();
    GlideCounters& counters = GlideRenderer::counters();
    ++counters.frames;
    counters.depth16Frames += m_depth16;
    m_renderer->paceFrame();
    glthread::post(m_renderer, [this]() {
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

/* No atlas to run out of: GL memory is the only limit. */
void ThrashRenderer::atlasFreeSpace(x86::reg32& texels, x86::reg32& wholeTiles, x86::reg32& total) const
{
    total = 4096 * 4096;
    texels = total;
    wholeTiles = 256;
}

/* What a texture takes of the Voodoo's texture memory: GlideRenderer's value,
 * so THRASH_talloc lays its addresses out as before -- each address is the
 * key of one texture here. */
x86::reg32 ThrashRenderer::getTextureMemSize(x86::reg32, x86::reg32, TextureFormat)
{
    return sizeof(void*);
}

void ThrashRenderer::setZWrite(bool enable)
{
    if (enable != m_zBuffer)
    {
        flush();
        m_zBuffer = enable;
    }
}

void ThrashRenderer::setZTest(bool enable)
{
    if (enable != m_zTest)
    {
        flush();
        m_zTest = enable;
    }
}

void ThrashRenderer::setDepthAlways(bool always)
{
    if (always != m_depthAlways)
    {
        flush();
        m_depthAlways = always;
    }
}

void ThrashRenderer::setAlphaBlendDst(AlphaBlend method)
{
    if (m_alphaBlend != (method == AB_1mSrcAlpha))
    {
        flush();
        m_alphaBlend = method == AB_1mSrcAlpha;
    }
}

// The target holds eight bits a channel: nothing to dither.
void ThrashRenderer::setDither(bool)
{
}

void ThrashRenderer::setCullMode(x86::reg32 mode)
{
    const bool enable = mode != 0;
    if (enable != m_cull || mode != m_cullMode)
    {
        flush();
        m_cull = enable;
        m_cullMode = mode;
    }
}

void ThrashRenderer::setChromakeyMode(bool enable)
{
    if (enable != m_chromaKey)
    {
        flush();
        m_chromaKey = enable;
    }
}

/* Each batch is drawn with one sampler: a change ends the batch. */
void ThrashRenderer::setTexClampMode(bool wrapS, bool wrapT)
{
    if (wrapS != m_wrapS || wrapT != m_wrapT)
    {
        flush();
        m_wrapS = wrapS;
        m_wrapT = wrapT;
    }
}

void ThrashRenderer::setChromakeyValue(x86::reg32 color)
{
    m_chromaKeyColor = color;
}

void ThrashRenderer::setClipWindow(x86::reg32 minX, x86::reg32 minY, x86::reg32 maxX, x86::reg32 maxY)
{
    if (minX != m_clipMinX || minY != m_clipMinY || maxX != m_clipMaxX || maxY != m_clipMaxY)
    {
        flush();
        m_clipMinX = minX;
        m_clipMinY = minY;
        m_clipMaxX = maxX;
        m_clipMaxY = maxY;
    }
}

void ThrashRenderer::setGamma(float correction)
{
    if (correction != m_gamma)
    {
        flush();
        m_gamma = correction;
    }
}

void ThrashRenderer::setColorFactor(float colorFactor)
{
    m_colorFactor = colorFactor;
}

void ThrashRenderer::setAlphaFactor(float alphaFactor)
{
    m_alphaFactor = alphaFactor;
}

// Read as each triangle is taken (see GlideRenderer::setFogMode).
void ThrashRenderer::setFogMode(x86::reg32 mode)
{
    m_fogMode = mode;
}

void ThrashRenderer::setFogColor(x86::reg32 color)
{
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

// A repeat of the table in force changes nothing (see GlideRenderer::setFogTable).
void ThrashRenderer::setFogTable(const x86::reg8* table)
{
    if (m_fogTableValid && std::memcmp(table, m_fogTableRaw, sizeof m_fogTableRaw) == 0)
        return;
    ++GlideRenderer::counters().fogTables;
    flush();
    std::memcpy(m_fogTableRaw, table, sizeof m_fogTableRaw);
    m_fogTableValid = true;
    std::array<float, 64> factors;
    for (int i = 0; i < 64; ++i)
        factors[i] = float(table[i]) / 255.f;
    m_fogTables.push_back(factors);
    m_fogTableIndex = int(m_fogTables.size()) - 1;
}

void ThrashRenderer::setAlphaTestRef(x86::reg32 value)
{
    const float ref = float(value & 0xff) / 255.f;
    if (ref != m_alphaTestRef)
    {
        flush();
        m_alphaTestRef = ref;
    }
}

/* The texture downloaded to this address, or the one-texel one downloaded to 0
 * when the window opened, for an address nothing was downloaded to. */
void ThrashRenderer::setTexture(x86::reg32, x86::reg32 address)
{
    auto found = m_textures.find(address);
    if (found == m_textures.end())
        found = m_textures.find(0);
    if (found == m_textures.end())
        return;
    if (found->second.index != m_texture)
    {
        flush();
        m_texture = found->second.index;
        m_textureLevels = found->second.levels;
        m_textureFormat = found->second.format;
    }
    found->second.useStamp = m_renderStamp;
}

/* The game's texels as they are, every level it supplies: 565 and 4444 in
 * formats of their own (4444 with its channels put right by the texture's
 * swizzle), 8888 as bytes swizzled from B, G, R, A, and only 1555 turned
 * round a bit into OpenGL's 5551.  GlideRenderer widened every one of them to
 * RGBA8 on the CPU first.  The GL thread gives the texture storage of that
 * size, levels and format, or keeps what it had when they are the same. */
void ThrashRenderer::setTextureData(x86::reg32, x86::reg32 address, const void* data, x86::reg32 largeMipmap,
                                    x86::reg32 smallMipmap, TextureFormat format)
{
    const x86::reg32 size = 256 >> largeMipmap;
    const x86::reg32 smallest = 256 >> smallMipmap;
    const x86::reg32 texelBytes = format == TF_ARGB_8888 ? 4 : 2;
    x86::reg32 levels = 0;
    std::size_t texels = 0;
    for (x86::reg32 at = size; at >= smallest && at; at >>= 1, ++levels)
        texels += std::size_t(at) * at;
    if (!levels)
        return;

    auto found = m_textures.find(address);
    if (found == m_textures.end())
    {
        Texture texture = { m_nextTexture++, 0, 0, 0, 0 };
        found = m_textures.emplace(address, texture).first;
    }
    Texture& texture = found->second;
    /* Triangles queued with this texture are drawn before it changes: they
     * were the old picture's on the Voodoo, rasterised as they came. */
    if (texture.useStamp == m_renderStamp && m_vertexCount)
        renderPending();
    // The chroma key's comparison (keyed() in the shader) by format.
    const int kind = format == TF_RGB_565 ? 1 : format == TF_ARGB_1555 ? 2 : format == TF_ARGB_4444 ? 3 : 0;
    const bool fresh = texture.size != size || texture.levels != levels || texture.format != kind;
    texture.size = size;
    texture.levels = levels;
    texture.format = kind;
    if (texture.index == m_texture)
    {
        m_textureLevels = levels;
        m_textureFormat = kind;
    }

    std::vector<x86::reg8> copied(static_cast<const x86::reg8*>(data),
                                  static_cast<const x86::reg8*>(data) + texels * texelBytes);
    if (format == TF_ARGB_1555)
    {
        // A1 R5 G5 B5 to R5 G5 B5 A1: the alpha bit from the top to the bottom.
        for (std::size_t i = 0; i < texels; ++i)
        {
            const x86::reg16 p = x86::reg16(copied[i * 2] | copied[i * 2 + 1] << 8);
            const x86::reg16 q = x86::reg16(p << 1 | p >> 15);
            copied[i * 2] = x86::reg8(q);
            copied[i * 2 + 1] = x86::reg8(q >> 8);
        }
    }
    const x86::reg32 index = texture.index;
    glthread::post(m_renderer, [this, index, size, levels, format, fresh, copied = std::move(copied)]() mutable {
        if (fresh && index < m_glTextures.size() && m_glTextures[index])
        {
            glDeleteTextures(1, &m_glTextures[index]);
            m_glTextures[index] = 0;
        }
        upload(index, size, levels, format, copied);
    });
}

void ThrashRenderer::upload(x86::reg32 index, x86::reg32 size, x86::reg32 levels, TextureFormat format,
                            std::vector<x86::reg8>& texels)
{
    GLenum internal = GL_RGBA8, layout = GL_RGBA, type = GL_UNSIGNED_BYTE;
    x86::reg32 texelBytes = 2;
    switch (format)
    {
    case TF_RGB_565:
        internal = GL_RGB565;
        layout = GL_RGB;
        type = GL_UNSIGNED_SHORT_5_6_5;
        break;
    case TF_ARGB_1555:
        internal = GL_RGB5_A1;
        type = GL_UNSIGNED_SHORT_5_5_5_1;
        break;
    case TF_ARGB_4444:
        internal = GL_RGBA4;
        type = GL_UNSIGNED_SHORT_4_4_4_4;
        break;
    default:
        texelBytes = 4;
        break;
    }
    if (index >= m_glTextures.size())
        m_glTextures.resize(index + 1, 0);
    GLuint& name = m_glTextures[index];
    if (!name)
    {
        glGenTextures(1, &name);
        glBindTexture(GL_TEXTURE_2D, name);
        glTexStorage2D(GL_TEXTURE_2D, GLsizei(levels), internal, GLsizei(size), GLsizei(size));
        if (format == TF_ARGB_4444)
        {
            // OpenGL reads A R G B as R G B A: each channel from the next one along.
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_GREEN);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_BLUE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_ALPHA);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);
        }
        else if (format == TF_ARGB_8888)
        {
            // B, G, R, A in memory: red and blue change places.
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_BLUE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
        }
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, name);
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    const x86::reg8* level = texels.data();
    for (x86::reg32 at = size, lod = 0; lod < levels; ++lod, at >>= 1)
    {
        glTexSubImage2D(GL_TEXTURE_2D, GLint(lod), 0, 0, GLsizei(at), GLsizei(at), layout, type, level);
        level += std::size_t(at) * at * texelBytes;
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

/* THRASH_treset: every texture but the window's one-texel one given up --
 * after what is queued is drawn, which may still use them. */
void ThrashRenderer::resetTextures()
{
    renderPending();
    std::vector<x86::reg32> released;
    for (auto it = m_textures.begin(); it != m_textures.end();)
    {
        if (it->first == 0)
        {
            ++it;
            continue;
        }
        released.push_back(it->second.index);
        it = m_textures.erase(it);
    }
    glthread::post(m_renderer, [this, released]() {
        for (const x86::reg32 index : released)
            if (index < m_glTextures.size() && m_glTextures[index])
            {
                glDeleteTextures(1, &m_glTextures[index]);
                m_glTextures[index] = 0;
            }
    });
}

/* Far wrapped texture coordinates brought back by whole repeats, as
 * GlideRenderer does (texCoordRebase there; NFS_TEXCOORD_REBASE=0 too). */
static bool rebaseOn()
{
    static const bool on = []() {
        const char* value = SDL_getenv("NFS_TEXCOORD_REBASE");
        return !value || SDL_strcmp(value, "0") != 0;
    }();
    return on;
}

static void rebase(float coord[3], const float oow[3])
{
    const float limit = 1024.f;
    if (!(std::fabs(coord[0]) > limit * oow[0] || std::fabs(coord[1]) > limit * oow[1]
          || std::fabs(coord[2]) > limit * oow[2]))
        return;
    if (!(oow[0] > 0.f && oow[1] > 0.f && oow[2] > 0.f))
        return;
    const double lowest = std::min(std::min(double(coord[0]) / oow[0], double(coord[1]) / oow[1]),
                                   double(coord[2]) / oow[2]);
    const double shift = std::floor(lowest / 256.0) * 256.0;
    for (int i = 0; i < 3; ++i)
        coord[i] = float(double(coord[i]) - shift * double(oow[i]));
}

void ThrashRenderer::drawTriangle(const GrVertex* a, const GrVertex* b, const GrVertex* c)
{
    auto corner = [](const GrVertex* in) {
        return Corner{ in->x, in->y, in->ooz, in->oow, in->tmuvtx[0].sow, in->tmuvtx[0].tow,
                       x86::reg32(x86::reg8(in->r)) | x86::reg32(x86::reg8(in->g)) << 8
                       | x86::reg32(x86::reg8(in->b)) << 16 | x86::reg32(x86::reg8(in->a)) << 24 };
    };
    drawCorners(corner(a), corner(b), corner(c));
}

/* The native THRASH driver's triangles come here without a GrVertex between
 * the game's vertices and this buffer (glide2x::direct::thrashRenderer). */
void ThrashRenderer::drawCorners(const Corner& a, const Corner& b, const Corner& c)
{
    ThrashVertex* out = reserveTriangle();
    const Corner* const corners[3] = { &a, &b, &c };
    for (int i = 0; i < 3; ++i)
    {
        const Corner* in = corners[i];
        out[i].x = in->x;
        out[i].y = in->y;
        out[i].z = in->ooz;
        out[i].color = in->color;
        out[i].u = in->sow;
        out[i].v = in->tow;
        out[i].oow = in->oow;
    }
    finishTriangle(out);
}

void ThrashRenderer::finishTriangle(ThrashVertex* out)
{
    const bool fogged = (m_fogMode & 0x3) == 0x2;
    const x86::reg8 combine[4] = { x86::reg8(m_colorFactor != 0.f), x86::reg8(m_alphaFactor != 0.f), 0,
                                   x86::reg8(fogged) };
    if ((m_wrapS || m_wrapT) && rebaseOn())
    {
        const float oow[3] = { out[0].oow, out[1].oow, out[2].oow };
        if (m_wrapS)
        {
            float sow[3] = { out[0].u, out[1].u, out[2].u };
            rebase(sow, oow);
            for (int i = 0; i < 3; ++i)
                out[i].u = sow[i];
        }
        if (m_wrapT)
        {
            float tow[3] = { out[0].v, out[1].v, out[2].v };
            rebase(tow, oow);
            for (int i = 0; i < 3; ++i)
                out[i].v = tow[i];
        }
    }
    for (int i = 0; i < 3; ++i)
        std::memcpy(out[i].combine, combine, sizeof combine);
    m_vertexCount += 3;
    GlideCounters& counters = GlideRenderer::counters();
    ++counters.triangles;
    if (fogged)
        ++counters.fogged;
}

}
