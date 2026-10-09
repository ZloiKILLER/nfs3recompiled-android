#include <new>
#include <lib/renderer.h>
#include <lib/window.h>
#include <lib/gamepad.h>
#include <SDL3/SDL.h>
#include <lib/glcompat.h>
#include <lib/glfuncs.h>
#include <lib/glthread.h>
#include <array>
#include <atomic>
#include <vector>
#ifdef __ANDROID__
#include <dlfcn.h>
#include <unistd.h>
#endif

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

/* Picture adjustment is applied here, on the finished frame, rather than inside
 * the Glide renderer: one pass over the screen instead of work on every
 * fragment of every triangle, and it covers the 2D menus and the movies as well
 * as a race.  The game has a gamma of its own that reaches the Glide shader;
 * the two simply compose.  Above 1 gamma lifts the dark end hardest, which is
 * the point -- the night tracks are lit almost entirely by the car's own
 * headlights.
 *
 * Order matters and is mirrored exactly by the launcher's preview (see
 * ScreenAdjustment.transfer in the Java side), so what the player sees while
 * dragging a slider is what the game will draw: gamma first, then contrast
 * around mid grey, then brightness as an offset. */
static const char g_blitFragmentShader[] =
"in vec2 v_texCoord;"
"layout (location=0) out vec4 o_color;"
"uniform sampler2D u_texture;"
"uniform float u_gamma;"
"uniform float u_brightness;"
"uniform float u_contrast;"
"void main()"
"{"
"    vec4 texel = texture(u_texture, v_texCoord);"
"    vec3 c = pow(max(texel.rgb, vec3(0.0)), vec3(1.0 / max(u_gamma, 0.001)));"
"    c = (c - vec3(0.5)) * u_contrast + vec3(0.5) + vec3(u_brightness);"
"    o_color = vec4(clamp(c, vec3(0.0), vec3(1.0)), texel.a);"
"}";

/* Read once each: the launcher sets them before the game starts, the way it
 * does the frame cap and the orientation.  The neutral value is what an unset
 * or unparseable variable means, so a missing launcher leaves the picture
 * exactly as the game drew it. */
static float envFloat(const char* name, float neutral, float low, float high)
{
    const char* value = SDL_getenv(name);
    const float parsed = value && *value ? float(SDL_atof(value)) : neutral;
    return (parsed >= low && parsed <= high) ? parsed : neutral;
}

static float displayGamma()
{
    static const float s_gamma = envFloat("NFS_GAMMA", 1.f, 0.05f, 10.f);
    return s_gamma;
}

static float displayBrightness()
{
    static const float s_brightness = envFloat("NFS_BRIGHTNESS", 0.f, -1.f, 1.f);
    return s_brightness;
}

static float displayContrast()
{
    static const float s_contrast = envFloat("NFS_CONTRAST", 1.f, 0.05f, 10.f);
    return s_contrast;
}

/* A context is created current on the thread that creates it, and a window's
 * surface can be current on one thread only: the GL thread lets go of its
 * context first. */
static SDL_GLContext createContext(SDL_Window* window)
{
    glthread::release();
    return SDL_GL_CreateContext(window);
}

Renderer::Renderer(WinApplication* application, Window *window)
    :   m_application(application)
    ,   m_window(window)
    ,   m_renderer(createContext(m_window->m_window))
    ,   m_blitProgram(0)
    ,   m_blitGammaUniform(-1)
    ,   m_blitBrightnessUniform(-1)
    ,   m_blitContrastUniform(-1)
    ,   m_blitVertexArray(0)
    ,   m_blitVertexBuffer(0)
    ,   m_videoMemory(new MemMap(800*600*2*2)) // double buffer 16 bits 800x600
    ,   m_currentBuffer(0)
    ,   m_depth(16)
    ,   m_swapInterval(0)
    ,   m_frameNs(0)
    ,   m_nextFrameNs(0)
    ,   m_frameStartNs(0)
    ,   m_presentNs(0)
    ,   m_convertBuffer(nullptr)
    ,   m_frame565(0)
    ,   m_frameRead(0)
    ,   m_frameDraw(0)
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
    m_blitBrightnessUniform = glGetUniformLocation(m_blitProgram, "u_brightness");
    m_blitContrastUniform = glGetUniformLocation(m_blitProgram, "u_contrast");
    glUseProgram(0);
    glBindVertexArray(0);
}

Renderer::~Renderer()
{
    SDL_RemoveEventWatch(keyboardEventWatch, nullptr);
    glthread::release();
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
    const x86::reg32 width = w, height = h;
    glthread::post(this, [this, width, height]() {
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glDisable(GL_DITHER);
        /* Sized internal format, deliberately, and eight bits per channel for
         * both guest depths.  This used to ask for the unsized GL_RGB, which
         * each implementation is free to resolve as it likes: desktop GL picked
         * 8 bits per channel while GLES on Mali picked a genuine RGB565 -- 32
         * colour levels per channel instead of 256.  Everything the Glide
         * renderer draws lands in this texture, so on the phone the whole 3D
         * image was being quantised to 16 bits and then dithered on top, which
         * is invisible in a still frame and crawls as soon as anything moves.
         * Measured directly: "render target R8 G8 B8" on Windows against
         * "R5 G6 B5" on the device. */
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        /* Scratch for the pixel conversion, sized once per mode change.
         * Needed for both guest depths now: the target is RGBA8, and neither
         * the 16-bit nor the palettised guest buffer can be handed to GL
         * directly. */
        free(m_convertBuffer);
        m_convertBuffer = reinterpret_cast<x86::reg8*>(malloc(size_t(width) * height * 4));

        /* The 16-bit guest frame goes to the GPU as it is, into a texture of
         * its own format, and the GPU copies it into the target, widening it
         * to eight bits a channel on the way (update()). */
        if (!m_frame565)
        {
            glGenTextures(1, &m_frame565);
            glGenFramebuffers(1, &m_frameRead);
            glGenFramebuffers(1, &m_frameDraw);
        }
        glBindTexture(GL_TEXTURE_2D, m_frame565);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB565, width, height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    });
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

/* The pads are read on the game's thread, the frame goes to the GL thread, and
 * the game waits only if that is still a frame behind -- that wait now stands
 * for the time present() used to spend in the swap (paceFrame). */
void Renderer::present()
{
    tick("present");
    Gamepad::update();
    const x86::reg32 width = m_width, height = m_height;
    glthread::post(this, [this, width, height]() { presentFrame(width, height); });
    m_presentNs += glthread::presentPosted(1);
}

void Renderer::presentFrame(x86::reg32 width, x86::reg32 height)
{
    const Uint64 presentStart = SDL_GetTicksNS();
    glBindTexture(GL_TEXTURE_2D, m_texture);
    int w, h;
    SDL_GetWindowSizeInPixels(m_window->m_window, &w, &h);
    SDL_LockSpinlock(&m_viewportLock);

    /* The letterbox math below only ever changes when the window's pixel
     * size does -- window resizes/rotations are rare compared to how often
     * present() runs (every game frame), and phase-2 touch input reads the
     * same rect via getViewportRect(), so it is worth keeping it cheap to
     * call from there too rather than recomputing on every present(). */
    if (w != m_lastWindowW || h != m_lastWindowH)
    {
        float gameAspect = float(width) / float(height);
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
    const int vpX = m_vpX, vpY = m_vpY + m_keyboardShift, vpW = m_vpW, vpH = m_vpH;
    SDL_UnlockSpinlock(&m_viewportLock);

    glViewport(0, 0, w, h);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    /* Render the game texture into the aspect-ratio-correct viewport.  The quad
     * is in NDC, so the letterbox rectangle is expressed purely by the viewport
     * and no projection matrix is needed. */
    glViewport(vpX, vpY, vpW, vpH);

    /* The Glide renderer leaves depth test and blending enabled; neither makes
     * sense for the blit, and it re-sets both per draw call. */
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    glUseProgram(m_blitProgram);
    if (m_blitGammaUniform >= 0)
        glUniform1f(m_blitGammaUniform, displayGamma());
    if (m_blitBrightnessUniform >= 0)
        glUniform1f(m_blitBrightnessUniform, displayBrightness());
    if (m_blitContrastUniform >= 0)
        glUniform1f(m_blitContrastUniform, displayContrast());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glBindVertexArray(m_blitVertexArray);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindVertexArray(0);
    glUseProgram(0);
    glEnable(GL_BLEND);

    glFlush();
    maybeGrabFrame(w, h);
    if (!SDL_GL_SwapWindow(m_window->m_window))
    {
        static unsigned s_failures = 0;
        if (++s_failures <= 8u || (s_failures % 600u) == 0u)
            SDL_Log("[GFX] swap failed (%u): %s", s_failures, SDL_GetError());
    }
    /* The single point where a frame actually reaches the screen, for both the
     * DirectDraw/2D menu path and the Glide path (GlideRenderer::swap() ends
     * up here too).  Every message-trace line carries this count, which is how
     * "the game was told about the key" gets tied to "a new frame came out". */
    NFS_MSG_FRAME();
    /* Mostly waiting: the window's clear on the buffer coming back from the
     * display, the swap on the GPU.  Not work a faster core would shorten.
     * On the game's thread only; on the GL thread present() counts the wait. */
    if (!glthread::on())
        m_presentNs += SDL_GetTicksNS() - presentStart;
    glthread::presentDone();
}

namespace
{
std::atomic<Renderer*>* activeSlot()
{
    static std::atomic<Renderer*> slot{nullptr};
    return &slot;
}
}

Renderer* Renderer::active()
{
    return activeSlot()->load();
}

void Renderer::presentMovieFrame(std::vector<x86::reg8> rgba, x86::reg32 w, x86::reg32 h,
                                 int x, int y, int dw, int dh)
{
    const x86::reg32 width = m_width, height = m_height;
    glthread::post(this, [this, w, h, x, y, dw, dh, width, height, rgba = std::move(rgba)]() {
        if (!m_frameRead || !m_frameDraw)
            return;
        if (!m_movieTexture)
            glGenTextures(1, &m_movieTexture);
        glBindTexture(GL_TEXTURE_2D, m_movieTexture);
        if (w != m_movieWidth || h != m_movieHeight)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, GLsizei(w), GLsizei(h), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            m_movieWidth = w;
            m_movieHeight = h;
        }
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GLsizei(w), GLsizei(h), GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
        /* Into the picture's texture: black, and the frame scaled onto its
         * place by the GPU.  Both textures have their first row at the top,
         * which a framebuffer blit keeps. */
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_frameRead);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_movieTexture, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frameDraw);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
        glDisable(GL_SCISSOR_TEST);
        glViewport(0, 0, GLsizei(width), GLsizei(height));
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        glBlitFramebuffer(0, 0, GLint(w), GLint(h), x, y, x + dw, y + dh, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    });
    present();
}

void Renderer::update()
{
    tick("update");
    activeSlot()->store(this);
    /* The guest's frame as it is now: the game writes the next one into the
     * same memory while the GL thread is still converting this one. */
    const x86::reg32 width = m_width, height = m_height;
    const bool deep = m_depth == 16;
    const x86::reg8* front = &m_application->getMemory<x86::reg8>(getFrontBuffer());
    std::vector<x86::reg8> frame(front, front + size_t(width) * height * (deep ? 2 : 1));
    std::array<x86::reg32, 256> palette;
    if (!deep)
        std::copy(m_colorPalette, m_colorPalette + 256, palette.begin());
    glthread::post(this, [this, width, height, deep, frame = std::move(frame), palette]() {
        if (!m_convertBuffer)
            return;
        if (deep && m_frame565)
        {
            /* The guest's 5:6:5 frame uploaded as it is, and copied into the
             * RGBA8 target by the GPU -- which widens five bits to eight as the
             * conversion loop below did, to within a level, white staying
             * white.  Menus
             * and movies used to cost the GL thread that loop over every
             * pixel of every frame. */
            glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
            glBindTexture(GL_TEXTURE_2D, m_frame565);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, frame.data());
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_frameRead);
            glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_frame565, 0);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frameDraw);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
            glDisable(GL_SCISSOR_TEST);
            glBlitFramebuffer(0, 0, GLint(width), GLint(height), 0, 0, GLint(width), GLint(height),
                              GL_COLOR_BUFFER_BIT, GL_NEAREST);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            return;
        }
        glBindTexture(GL_TEXTURE_2D, m_texture);
        /* Storage and filtering are set up once in setVideoMode(): they are
         * texture object state, so re-sending them every frame is pure
         * overhead, and glTexImage2D would additionally reallocate the storage
         * each time. */
        x86::reg8* screenData = m_convertBuffer;
        if (deep)
        {
            /* Expand the guest 5:6:5 buffer to the RGBA8 the target now uses.
             * The low bits are replicated rather than zero-filled so white
             * stays white instead of drifting to 248,252,248.  Only the
             * DirectDraw path comes through here -- menus and the intro movie
             * -- because the Glide renderer draws into this texture directly,
             * so a race pays nothing for this loop. */
            const x86::reg16* srcData = reinterpret_cast<const x86::reg16*>(frame.data());
            for (x86::reg32 i = 0; i < width*height; ++i)
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
        }
        else
        {
            /* Palette entries keep the layout the old GL_UNSIGNED_INT_8_8_8_8
             * upload implied -- first component in the most significant byte
             * -- so unpack them into R,G,B,A memory order and upload as
             * GL_UNSIGNED_BYTE, which is both endian-independent and available
             * in GLES.  The destination is a persistent buffer: this used to
             * malloc and free ~1.9 MB every single frame. */
            const x86::reg8* srcData = frame.data();
            for (x86::reg32 i = 0; i < width*height; ++i)
            {
                const x86::reg32 c = palette[srcData[i]];
                screenData[i*4 + 0] = x86::reg8(c >> 24);
                screenData[i*4 + 1] = x86::reg8(c >> 16);
                screenData[i*4 + 2] = x86::reg8(c >> 8);
                screenData[i*4 + 3] = x86::reg8(c);
            }
        }
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                        GL_RGBA, GL_UNSIGNED_BYTE, screenData);
    });
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
        /* This used to be SDL_GL_SetSwapInterval(0) -- presumably to push the
         * unlocked front buffer out without waiting for a refresh.  But the
         * interval is context-global and nothing ever restored it, so one
         * DirectDraw unlock left the whole process running without vsync for
         * good.  Pace this present like every other one instead. */
        ensureFramePacing();
        update();
    }
}

/* The frame rate the player chose, 30 or 60 (NFS_FPS_CAP), kept by the clock.
 *
 * This used to ask the driver to hold each present for refresh/cap refreshes.
 * Android takes the request, reports success, and holds each present for one
 * refresh all the same -- its Surface allows no swap interval above one -- so
 * nothing capped anything: a phone with a 120 Hz panel drew the game as fast
 * as it could, 30 or 60 alike, and ran hot doing it.  Now vsync stays at one
 * refresh and each frame waits for its time before it goes out (paceFrame);
 * on Android 11 and later the activity asks the display for the same rate
 * (NFS3Activity), so a 60 on a 120 Hz panel lands on every refresh of a 60 Hz
 * one instead of every other of a 120 Hz one.
 *
 * Only once: the interval belongs to the surface, and a new surface after the
 * app returns starts at one, which is what is wanted anyway. */
void Renderer::ensureFramePacing()
{
    if (m_swapInterval != 0)
        return;
    glthread::post(this, []() { SDL_GL_SetSwapInterval(1); });
    m_swapInterval = 1;
    const char* capEnv = SDL_getenv("NFS_FPS_CAP");
    const int cap = capEnv ? SDL_atoi(capEnv) : 30;
    /* 0: no cap, the display's own refresh the only pace. */
    m_frameNs = cap > 0 ? SDL_NS_PER_SECOND / Uint64(cap) : 0;
    if (cap > 0)
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "frame pacing: vsync, %d frames a second by the clock", cap);
    else
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "frame pacing: vsync, no cap");
}

#ifdef __ANDROID__
namespace
{

/* How long each frame's work took, told to Android (ADPF, the performance hint
 * API: Android 13, libandroid.so) against how long a frame may take.  The phone
 * runs the game thread on a big core it holds at half its clock; told a frame
 * went over, it raises that core's clock for the thread, and told they come in
 * early, lowers it -- instead of guessing from load.  Looked up at run time:
 * the app starts on Android 8, where none of it exists and nothing happens.
 * NFS_PERF_HINT=0 turns it off. */
class PerformanceHint
{
public:
    void report(Uint64 targetNs, Uint64 workNs)
    {
        if (!m_tried)
            start(targetNs);
        if (!m_session)
            return;
        if (targetNs != m_target && m_update)
        {
            m_update(m_session, int64_t(targetNs));
            m_target = targetNs;
        }
        if (workNs > 0)
            m_report(m_session, int64_t(workNs));
    }

private:
    typedef void* (*GetManager)();
    typedef void* (*CreateSession)(void*, const int32_t*, size_t, int64_t);
    typedef int (*ReportWork)(void*, int64_t);
    typedef int (*UpdateTarget)(void*, int64_t);

    void start(Uint64 targetNs)
    {
        m_tried = true;
        const char* setting = SDL_getenv("NFS_PERF_HINT");
        if (setting && SDL_strcmp(setting, "0") == 0)
            return;
        void* library = dlopen("libandroid.so", RTLD_NOW | RTLD_LOCAL);
        GetManager getManager = library ? GetManager(dlsym(library, "APerformanceHint_getManager")) : nullptr;
        CreateSession create = library ? CreateSession(dlsym(library, "APerformanceHint_createSession")) : nullptr;
        m_report = library ? ReportWork(dlsym(library, "APerformanceHint_reportActualWorkDuration")) : nullptr;
        m_update = library ? UpdateTarget(dlsym(library, "APerformanceHint_updateTargetWorkDuration")) : nullptr;
        void* manager = getManager ? getManager() : nullptr;
        const int32_t thread = int32_t(gettid());
        if (manager && create && m_report)
            m_session = create(manager, &thread, 1, int64_t(targetNs));
        m_target = targetNs;
        /* Which step it stopped at: no API before Android 13, no manager, or a
         * device whose power HAL takes no hint sessions (an S20+ on Android 13). */
        const char* state = m_session ? "on"
                          : !create ? "not in this Android"
                          : !manager ? "no manager"
                          : "not supported by this device";
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[PERF] performance hint %s (frame %.1f ms, thread %d)",
                    state, double(targetNs) / 1e6, int(thread));
    }

    bool m_tried = false;
    void* m_session = nullptr;
    ReportWork m_report = nullptr;
    UpdateTarget m_update = nullptr;
    Uint64 m_target = 0;
};

PerformanceHint s_performanceHint;

}
#endif

void Renderer::paceFrame()
{
    const Uint64 now = SDL_GetTicksNS();
#ifdef __ANDROID__
    /* The work since the last frame went out, less the waiting in present(). */
    if (m_frameStartNs && now > m_frameStartNs)
    {
        const Uint64 spent = now - m_frameStartNs;
        s_performanceHint.report(m_frameNs ? m_frameNs : SDL_NS_PER_SECOND / 60,
                                 spent > m_presentNs ? spent - m_presentNs : 0);
    }
#endif
    m_presentNs = 0;
    if (m_frameNs)
    {
        /* The first frame, or one that came a whole period late or more: count
         * from now, rather than rush the frames after it to catch up. */
        if (!m_nextFrameNs || now >= m_nextFrameNs + m_frameNs)
            m_nextFrameNs = now;
        else if (now < m_nextFrameNs)
            SDL_DelayNS(m_nextFrameNs - now);
        m_nextFrameNs += m_frameNs;
    }
    m_frameStartNs = SDL_GetTicksNS();
}

void Renderer::swap()
{
    tick("swap");
    ensureFramePacing();

    m_currentBuffer = 1 - m_currentBuffer;
    paceFrame();
    update();
}

}
