#include <new>
#include <lib/renderer.h>
#include <lib/window.h>
#include <lib/gamepad.h>
#include <SDL3/SDL.h>
#include <lib/glcompat.h>
#include <lib/glfuncs.h>

namespace
{

/* Frame grabber.  Set NFS_SCREENSHOT to a .bmp path and the renderer writes the
 * window contents there every NFS_SCREENSHOT_MS milliseconds (default 2000).
 * Reading the framebuffer from inside beats capturing the window from outside:
 * it does not care about z-order or DPI scaling on the desktop, and on Android
 * it works without adb screencap. */
void maybeGrabFrame(int width, int height)
{
    static const char* s_path = SDL_getenv("NFS_SCREENSHOT");
    if (!s_path || !*s_path || width <= 0 || height <= 0)
        return;

    static const Uint64 s_period = []() -> Uint64 {
        const char* v = SDL_getenv("NFS_SCREENSHOT_MS");
        const int ms = v ? SDL_atoi(v) : 0;
        return ms > 0 ? Uint64(ms) : 2000;
    }();

    static Uint64 s_last = 0;
    const Uint64 now = SDL_GetTicks();
    if (s_last && now - s_last < s_period)
        return;
    s_last = now;

    const size_t rowLen = size_t(width) * 3;
    const size_t padded = (rowLen + 3) & ~size_t(3);
    unsigned char* pixels = static_cast<unsigned char*>(malloc(size_t(height) * rowLen));
    if (!pixels)
        return;
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    SDL_IOStream* io = SDL_IOFromFile(s_path, "wb");
    if (io)
    {
        const Uint32 dataSize = Uint32(padded * size_t(height));
        Uint8 header[54] = {0};
        header[0] = 'B'; header[1] = 'M';
        const Uint32 fileSize = 54 + dataSize;
        const Uint32 offset = 54;
        const Uint32 dibSize = 40;
        const Sint32 w32 = width, h32 = height;
        const Uint16 planes = 1, bpp = 24;
        SDL_memcpy(header + 2,  &fileSize, 4);
        SDL_memcpy(header + 10, &offset,   4);
        SDL_memcpy(header + 14, &dibSize,  4);
        SDL_memcpy(header + 18, &w32,      4);
        SDL_memcpy(header + 22, &h32,      4);
        SDL_memcpy(header + 26, &planes,   2);
        SDL_memcpy(header + 28, &bpp,      2);
        SDL_memcpy(header + 34, &dataSize, 4);
        SDL_WriteIO(io, header, sizeof(header));

        /* glReadPixels returns bottom-up, which is exactly BMP's row order;
         * only RGB has to be swapped to BGR. */
        unsigned char* row = static_cast<unsigned char*>(malloc(padded));
        if (row)
        {
            SDL_memset(row, 0, padded);
            for (int y = 0; y < height; ++y)
            {
                const unsigned char* src = pixels + size_t(y) * rowLen;
                for (int x = 0; x < width; ++x)
                {
                    row[x*3 + 0] = src[x*3 + 2];
                    row[x*3 + 1] = src[x*3 + 1];
                    row[x*3 + 2] = src[x*3 + 0];
                }
                SDL_WriteIO(io, row, padded);
            }
            free(row);
        }
        SDL_CloseIO(io);
    }
    free(pixels);
}

#ifdef NFS_TRACE_MSG
struct PresentStats
{
    unsigned update, present, swap, unlock0, unlockN;
    Uint64   last;
};
PresentStats g_ps = {0,0,0,0,0,0};
void tick(const char* what)
{
    if      (!SDL_strcmp(what, "update"))   g_ps.update++;
    else if (!SDL_strcmp(what, "present"))  g_ps.present++;
    else if (!SDL_strcmp(what, "swap"))     g_ps.swap++;
    else if (!SDL_strcmp(what, "unlock0"))  g_ps.unlock0++;
    else                                     g_ps.unlockN++;
    const Uint64 now = SDL_GetTicks();
    if (now - g_ps.last >= 1000)
    {
        SDL_Log("[GFX] per sec: update=%u present=%u swap=%u unlock(0)=%u unlock(n)=%u",
                g_ps.update, g_ps.present, g_ps.swap, g_ps.unlock0, g_ps.unlockN);
        g_ps.update = g_ps.present = g_ps.swap = g_ps.unlock0 = g_ps.unlockN = 0;
        g_ps.last = now;
    }
}
#else
inline void tick(const char*) {}
#endif
}

namespace win32
{

/* Fraction of the picture's own height, not the window's: the game's dialog
 * sits at a fixed place in the 640x480 frame, so a fraction of the frame
 * behaves the same whatever the screen's shape.  Nineteen percent clears the
 * keyboard with the dialog's Ok and Cancel still on screen -- fifteen cleared
 * the field alone and cut the buttons off at the keyboard's edge.  The strip it
 * frees at the bottom is itself under the keyboard, so raising the picture
 * exposes nothing. */
static const float s_keyboardShiftFraction = 0.19f;

/* Written from the thread SDL raises the event on, read by present() on the
 * SDL thread and by the activity over JNI. */
static SDL_AtomicInt s_screenKeyboardVisible;

/* Both edges arrive here, which is why this watches SDL instead of asking
 * Android: SDL raises SHOWN from the task that opens the keyboard, and the
 * system back key routes through onNativeKeyboardFocusLost -> SDL_StopTextInput
 * -> HIDDEN.  A keyboard dismissed behind the game's back still reports. */
static bool SDLCALL keyboardEventWatch(void* /*userdata*/, SDL_Event* event)
{
    if (event->type == SDL_EVENT_SCREEN_KEYBOARD_SHOWN)
        SDL_SetAtomicInt(&s_screenKeyboardVisible, 1);
    else if (event->type == SDL_EVENT_SCREEN_KEYBOARD_HIDDEN)
        SDL_SetAtomicInt(&s_screenKeyboardVisible, 0);
    return true;
}


#if !NFS_GLES
void GLAPIENTRY errorCallback(GLenum /*source*/, GLenum type, GLuint /*id*/, GLenum severity, GLsizei /*length*/, const GLchar* message, const void* /*userParam*/)
{
    if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
    SDL_LogError(SDL_LOG_CATEGORY_RENDER, "OpenGL: %s type = 0x%x, severity = 0x%x, \"%s\"\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
}
PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;
#endif

static const char g_blitPreamble[] = NFS_GLSL_PREAMBLE;

/* Blits m_texture over the whole viewport.  The viewport is already set to the
 * aspect-correct letterbox rectangle, so the quad is plain NDC and needs no
 * matrix.  v is flipped because the game's framebuffer has row 0 at the top. */
static const char g_blitVertexShader[] =
"in vec2 a_position;"
"in vec2 a_texCoord;"
"out vec2 v_texCoord;"
"void main()"
"{"
"    v_texCoord = a_texCoord;"
"    gl_Position = vec4(a_position, 0.0, 1.0);"
"}";

/* Gamma is applied here, on the finished frame, rather than inside the Glide
 * renderer: one pass over the screen instead of work on every fragment of every
 * triangle, and it covers the 2D menus and the movies as well as a race.  The
 * game has a gamma of its own that reaches the Glide shader; the two simply
 * compose.  Above 1 this lifts the dark end hardest, which is the point -- the
 * night tracks are lit almost entirely by the car's own headlights. */
static const char g_blitFragmentShader[] =
"in vec2 v_texCoord;"
"layout (location=0) out vec4 o_color;"
"uniform sampler2D u_texture;"
"uniform float u_gamma;"
"void main()"
"{"
"    vec4 texel = texture(u_texture, v_texCoord);"
"    o_color = vec4(pow(max(texel.rgb, vec3(0.0)), vec3(1.0 / max(u_gamma, 0.001))), texel.a);"
"}";

/* Read once: the launcher sets it before the game starts, the way it does the
 * frame cap and the orientation.  1.0 leaves the picture exactly as the game
 * drew it, and is what an unset variable means. */
static float displayGamma()
{
    static const float s_gamma = []() {
        const char* value = SDL_getenv("NFS_GAMMA");
        const float parsed = value ? float(SDL_atof(value)) : 1.f;
        return (parsed > 0.05f && parsed < 10.f) ? parsed : 1.f;
    }();
    return s_gamma;
}

Renderer::Renderer(WinApplication* application, Window *window)
    :   m_application(application)
    ,   m_window(window)
    ,   m_renderer(SDL_GL_CreateContext(m_window->m_window))
    ,   m_blitProgram(0)
    ,   m_blitGammaUniform(-1)
    ,   m_blitVertexArray(0)
    ,   m_blitVertexBuffer(0)
    ,   m_videoMemory(new MemMap(800*600*2*2)) // double buffer 16 bits 800x600
    ,   m_currentBuffer(0)
    ,   m_depth(16)
    ,   m_swapInterval(0)
    ,   m_extraWaitMs(0)
    ,   m_convertBuffer(nullptr)
    ,   m_colorPalette()
    ,   m_lastWindowW(-1)
    ,   m_lastWindowH(-1)
    ,   m_vpX(0), m_vpY(0), m_vpW(0), m_vpH(0)
    ,   m_keyboardShift(0)
{
    SDL_SetAtomicInt(&s_screenKeyboardVisible, 0);
    SDL_AddEventWatch(keyboardEventWatch, nullptr);
    setCurrent();
    loadGlFunctions();
#if !NFS_GLES
    glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)SDL_GL_GetProcAddress("glDebugMessageCallback");
    if (glDebugMessageCallback)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(errorCallback, 0);
    }
#endif

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    initBlit();

    clearCurrent();
}

namespace
{

void logShader(GLuint shader, const char* what)
{
    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status)
        return;
    GLint maxLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLen);
    if (maxLen > 1)
    {
        GLchar* log = static_cast<GLchar*>(malloc(maxLen));
        GLsizei len = 0;
        glGetShaderInfoLog(shader, maxLen, &len, log);
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "%s: %s", what, log);
        free(log);
    }
}

void logProgram(GLuint program, const char* what)
{
    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status)
        return;
    GLint maxLen = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLen);
    if (maxLen > 1)
    {
        GLchar* log = static_cast<GLchar*>(malloc(maxLen));
        GLsizei len = 0;
        glGetProgramInfoLog(program, maxLen, &len, log);
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "%s: %s", what, log);
        free(log);
    }
}

}

void Renderer::initBlit()
{
    const GLchar* vsSrc[2] = { g_blitPreamble, g_blitVertexShader };
    const GLint vsLen[2] = { GLint(sizeof(g_blitPreamble) - 1), GLint(sizeof(g_blitVertexShader) - 1) };
    const GLchar* fsSrc[2] = { g_blitPreamble, g_blitFragmentShader };
    const GLint fsLen[2] = { GLint(sizeof(g_blitPreamble) - 1), GLint(sizeof(g_blitFragmentShader) - 1) };

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 2, vsSrc, vsLen);
    glCompileShader(vs);
    logShader(vs, "blit vertex shader");

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 2, fsSrc, fsLen);
    glCompileShader(fs);
    logShader(fs, "blit fragment shader");

    m_blitProgram = glCreateProgram();
    glAttachShader(m_blitProgram, vs);
    glAttachShader(m_blitProgram, fs);
    glLinkProgram(m_blitProgram);
    logProgram(m_blitProgram, "blit program");

    /* x, y, u, v -- triangle strip: top-left, bottom-left, top-right, bottom-right */
    static const float quad[16] = {
        -1.0f,  1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 1.0f,
    };

    glGenVertexArrays(1, &m_blitVertexArray);
    glBindVertexArray(m_blitVertexArray);
    glGenBuffers(1, &m_blitVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_blitVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    const GLint posLoc = glGetAttribLocation(m_blitProgram, "a_position");
    const GLint uvLoc  = glGetAttribLocation(m_blitProgram, "a_texCoord");
    if (posLoc >= 0)
    {
        glVertexAttribPointer(GLuint(posLoc), 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
        glEnableVertexAttribArray(GLuint(posLoc));
    }
    if (uvLoc >= 0)
    {
        glVertexAttribPointer(GLuint(uvLoc), 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (const void*)(2*sizeof(float)));
        glEnableVertexAttribArray(GLuint(uvLoc));
    }

    glUseProgram(m_blitProgram);
    const GLint texLoc = glGetUniformLocation(m_blitProgram, "u_texture");
    if (texLoc >= 0)
        glUniform1i(texLoc, 0);
    m_blitGammaUniform = glGetUniformLocation(m_blitProgram, "u_gamma");
    glUseProgram(0);
    glBindVertexArray(0);
}

Renderer::~Renderer()
{
    SDL_RemoveEventWatch(keyboardEventWatch, nullptr);
    free(m_convertBuffer);
    SDL_GL_DestroyContext(static_cast<SDL_GLContext>(m_renderer));
}

void Renderer::setCurrent()
{
    SDL_GL_MakeCurrent(m_window->m_window, static_cast<SDL_GLContext>(m_renderer));
}

void Renderer::clearCurrent()
{
    SDL_GL_MakeCurrent(m_window->m_window, nullptr);
}

void Renderer::setVideoMode(x86::reg32 w, x86::reg32 h, x86::reg32 bpp)
{
    // Guest LFB remains two-byte RGB565 for Glide; allocate for the actual mode.
    // This also covers the existing 1024x768 mode, beyond the old 800x600 buffer.
    const size_t bytes = size_t(w) * h * 2 * 2;
    if (!w || !h || bytes > 128u * 1024u * 1024u) throw std::bad_alloc();
    if (bytes > m_videoMemory->getBlockSize()) {
        MemMap* replacement = new MemMap(x86::reg32(bytes));
        delete m_videoMemory;
        m_videoMemory = replacement;
    }
    m_currentBuffer = 0;
    m_lastWindowW = m_lastWindowH = -1;
    m_width = w;
    m_height = h;
    m_depth = bpp;

    /* The per-frame path is glTexSubImage2D, so the storage has to be allocated
     * here and in the format update() will actually upload -- 5_6_5 for the
     * 16-bit surface, RGBA bytes for the palettised one. */
    setCurrent();
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glDisable(GL_DITHER);
    /* Sized internal format, deliberately, and eight bits per channel for both
     * guest depths.  This used to ask for the unsized GL_RGB, which each
     * implementation is free to resolve as it likes: desktop GL picked 8 bits
     * per channel while GLES on Mali picked a genuine RGB565 -- 32 colour
     * levels per channel instead of 256.  Everything the Glide renderer draws
     * lands in this texture, so on the phone the whole 3D image was being
     * quantised to 16 bits and then dithered on top, which is invisible in a
     * still frame and crawls as soon as anything moves.  Measured directly:
     * "render target R8 G8 B8" on Windows against "R5 G6 B5" on the device. */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    clearCurrent();

    /* Scratch for the pixel conversion, sized once per mode change.  Needed
     * for both guest depths now: the target is RGBA8, and neither the 16-bit
     * nor the palettised guest buffer can be handed to GL directly. */
    free(m_convertBuffer);
    m_convertBuffer = reinterpret_cast<x86::reg8*>(malloc(size_t(m_width) * m_height * 4));
}

void Renderer::updatePalette(x86::reg32 colorCount, const x86::reg32* colors)
{
    NFS2_ASSERT(colorCount <= 256);
    memcpy(m_colorPalette, colors, colorCount*4);
}

x86::reg32 Renderer::getFrontBuffer() const
{
    return m_videoMemory->getBlockStart() + m_currentBuffer * m_width * m_height * 2;
}

x86::reg32 Renderer::getBackBuffer() const
{
    return m_videoMemory->getBlockStart() + (1 - m_currentBuffer) * m_width * m_height * 2;
}

void Renderer::present()
{
    tick("present");
    Gamepad::updateKeys();
    glBindTexture(GL_TEXTURE_2D, m_texture);
    int w, h;
    SDL_GetWindowSizeInPixels(m_window->m_window, &w, &h);

    /* The letterbox math below only ever changes when the window's pixel
     * size does -- window resizes/rotations are rare compared to how often
     * present() runs (every game frame), and phase-2 touch input reads the
     * same rect via getViewportRect(), so it is worth keeping it cheap to
     * call from there too rather than recomputing on every present(). */
    if (w != m_lastWindowW || h != m_lastWindowH)
    {
        float gameAspect = float(m_width) / float(m_height);
        float windowAspect = float(w) / float(h);
        if (windowAspect > gameAspect)
        {
            m_vpH = h;
            m_vpW = int(h * gameAspect + 0.5f);
            m_vpX = (w - m_vpW) / 2;
            m_vpY = 0;
        }
        else
        {
            m_vpW = w;
            m_vpH = int(w / gameAspect + 0.5f);
            m_vpX = 0;
            m_vpY = (h - m_vpH) / 2;
        }
        m_lastWindowW = w;
        m_lastWindowH = h;
    }

    /* Recomputed every frame rather than cached with the letterbox: it follows
     * the keyboard, not the window size. */
    m_keyboardShift = SDL_GetAtomicInt(&s_screenKeyboardVisible)
                    ? int(m_vpH * s_keyboardShiftFraction + 0.5f)
                    : 0;

    glViewport(0, 0, w, h);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    /* Render the game texture into the aspect-ratio-correct viewport.  The quad
     * is in NDC, so the letterbox rectangle is expressed purely by the viewport
     * and no projection matrix is needed. */
    glViewport(m_vpX, m_vpY + m_keyboardShift, m_vpW, m_vpH);

    /* The Glide renderer leaves depth test and blending enabled; neither makes
     * sense for the blit, and it re-sets both per draw call. */
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    glUseProgram(m_blitProgram);
    if (m_blitGammaUniform >= 0)
        glUniform1f(m_blitGammaUniform, displayGamma());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glBindVertexArray(m_blitVertexArray);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindVertexArray(0);
    glUseProgram(0);
    glEnable(GL_BLEND);

    glFlush();
    maybeGrabFrame(w, h);
    SDL_GL_SwapWindow(m_window->m_window);
    /* The single point where a frame actually reaches the screen, for both the
     * DirectDraw/2D menu path and the Glide path (GlideRenderer::swap() ends
     * up here too).  Every message-trace line carries this count, which is how
     * "the game was told about the key" gets tied to "a new frame came out". */
    NFS_MSG_FRAME();
}

void Renderer::update()
{
    tick("update");
    glBindTexture(GL_TEXTURE_2D, m_texture);
    /* Storage and filtering are set up once in setVideoMode(): they are texture
     * object state, so re-sending them every frame is pure overhead, and
     * glTexImage2D would additionally reallocate the storage each time. */
    if (m_depth == 16 && m_convertBuffer)
    {
        /* Expand the guest 5:6:5 buffer to the RGBA8 the target now uses.  The
         * low bits are replicated rather than zero-filled so white stays white
         * instead of drifting to 248,252,248.  Only the DirectDraw path comes
         * through here -- menus and the intro movie -- because the Glide
         * renderer draws into this texture directly, so a race pays nothing
         * for this loop. */
        const x86::reg16* srcData = &m_application->getMemory<x86::reg16>(getFrontBuffer());
        x86::reg8* screenData = m_convertBuffer;
        for (x86::reg32 i = 0; i < m_width*m_height; ++i)
        {
            const x86::reg16 c = srcData[i];
            const x86::reg8 r = x86::reg8((c >> 11) & 0x1f);
            const x86::reg8 g = x86::reg8((c >> 5) & 0x3f);
            const x86::reg8 b = x86::reg8(c & 0x1f);
            screenData[i*4 + 0] = x86::reg8(r << 3 | r >> 2);
            screenData[i*4 + 1] = x86::reg8(g << 2 | g >> 4);
            screenData[i*4 + 2] = x86::reg8(b << 3 | b >> 2);
            screenData[i*4 + 3] = 0xff;
        }
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height,
                        GL_RGBA, GL_UNSIGNED_BYTE, screenData);
    }
    else if (m_convertBuffer)
    {
        /* Palette entries keep the layout the old GL_UNSIGNED_INT_8_8_8_8 upload
         * implied -- first component in the most significant byte -- so unpack
         * them into R,G,B,A memory order and upload as GL_UNSIGNED_BYTE, which
         * is both endian-independent and available in GLES.  The destination is
         * a persistent buffer: this used to malloc and free ~1.9 MB every single
         * frame. */
        const x86::reg8* srcData = &m_application->getMemory<x86::reg8>(getFrontBuffer());
        x86::reg8* screenData = m_convertBuffer;
        for (x86::reg32 i = 0; i < m_width*m_height; ++i)
        {
            const x86::reg32 c = m_colorPalette[srcData[i]];
            screenData[i*4 + 0] = x86::reg8(c >> 24);
            screenData[i*4 + 1] = x86::reg8(c >> 16);
            screenData[i*4 + 2] = x86::reg8(c >> 8);
            screenData[i*4 + 3] = x86::reg8(c);
        }
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height,
                        GL_RGBA, GL_UNSIGNED_BYTE, screenData);
    }
    present();
}

x86::reg32 Renderer::lock(x86::reg32 index)
{
    return index == 0 ? getFrontBuffer() : getBackBuffer();
}

void Renderer::unlock(x86::reg32 index)
{
    tick(index == 0 ? "unlock0" : "unlockN");
    if (index == 0)
    {
        setCurrent();
        /* This used to be SDL_GL_SetSwapInterval(0) -- presumably to push the
         * unlocked front buffer out without waiting for a refresh.  But the
         * interval is context-global and nothing ever restored it, so one
         * DirectDraw unlock left the whole process running without vsync for
         * good.  Pace this present like every other one instead. */
        ensureFramePacing();
        update();
        clearCurrent();
    }
}

/* The game is paced to 30 Hz.  Upstream got that by presenting the same frame
 * refresh_rate/30 times with vsync on, which on a 120 Hz phone panel costs four
 * full framebuffer uploads, four palette conversions and four presents for a
 * single game frame.  Ask the driver to hold each present for that many
 * refreshes instead: identical pacing, a quarter of the work.
 *
 * Negotiating once is not enough on its own, because the swap interval is
 * context-global and other paths used to overwrite it -- unlock() set it to 0
 * and nothing ever put it back, so after the first DirectDraw unlock the whole
 * 2D path (every menu) free-ran at whatever the hardware would produce.  That
 * was the port's main source of heat.  Every presenting path now calls this. */
void Renderer::ensureFramePacing()
{
    if (m_swapInterval != 0)
    {
        /* Already negotiated; just make sure nothing has changed it since. */
        SDL_GL_SetSwapInterval(m_swapInterval);
        return;
    }

    const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay());
    /* NFS_FPS_CAP overrides the 30 Hz target without a rebuild, which is the
     * only way to tell a cap that is too low apart from a machine that is
     * simply too slow. */
    const char* capEnv = SDL_getenv("NFS_FPS_CAP");
    const int cap = capEnv ? SDL_atoi(capEnv) : 30;
    const int wanted = (mode && cap > 0) ? int(mode->refresh_rate / float(cap)) : 2;
    const int interval = wanted > 1 ? wanted : 1;

    if (interval > 1 && SDL_GL_SetSwapInterval(interval))
    {
        m_swapInterval = interval;
        m_extraWaitMs = 0;
    }
    else
    {
        /* Driver refused an interval above one.  Upstream paced this by
         * presenting the same frame `interval` times, which costs that many
         * full texture uploads and presents for one game frame.  Present once
         * and wait out the remaining refreshes instead -- same frame rate, one
         * frame's worth of work. */
        SDL_GL_SetSwapInterval(1);
        m_swapInterval = 1;
        const float refresh = (mode && mode->refresh_rate > 1.0f) ? mode->refresh_rate : 60.0f;
        m_extraWaitMs = (interval > 1) ? int((interval - 1) * 1000.0f / refresh) : 0;
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "frame pacing: swap interval %d, extra wait %d ms",
                m_swapInterval, m_extraWaitMs);
}

void Renderer::swap()
{
    tick("swap");
    setCurrent();
    ensureFramePacing();

    m_currentBuffer = 1 - m_currentBuffer;
    update();
    clearCurrent();

    /* Only ever non-zero when the driver would not hold a present for more than
     * one refresh; see ensureFramePacing(). */
    if (m_extraWaitMs > 0)
    {
        SDL_Delay(Uint32(m_extraWaitMs));
    }
}

}
