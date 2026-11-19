#include <lib/window.h>
#include <lib/thread.h>
#include <lib/glplatform.h>
#include <SDL3/SDL.h>
#include <cstring>
#include <cstdarg>

namespace nfs
{

Uint32 g_gameMessageEvent = 0;
Uint32 g_portEvent        = 0;
Uint32 g_threadStopEvent  = 0;
Uint32 g_characterEvent   = 0;

static struct CustomEventRegistrar
{
    CustomEventRegistrar()
    {
        Uint32 base = SDL_RegisterEvents(4);
        g_gameMessageEvent = base + 0;
        g_portEvent        = base + 1;
        g_threadStopEvent  = base + 2;
        g_characterEvent   = base + 3;
    }
} s_customEventRegistrar;

#ifdef NFS_TRACE_MSG
unsigned g_traceFrames = 0;

void eventTrace(const char* fmt, ...)
{
    char buffer[512];
    va_list args;
    va_start(args, fmt);
    SDL_vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    SDL_Log("[MSG] f%u t%u %ums %s",
            g_traceFrames,
            unsigned(Thread::currentThreadId()),
            unsigned(SDL_GetTicks()),
            buffer);
}

bool eventTraceRateLimit(unsigned& hits)
{
    ++hits;
    return hits <= 8u || (hits % 64u) == 0u;
}
#endif


namespace
{

/* GL attributes select the pixel format, so they must be set before the window
 * exists.  Upstream set none and took SDL's default, which is a desktop GL
 * context -- on Android that fails outright. */
SDL_WindowFlags requestGlContext()
{
#if NFS_GLES
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    /* Android owns the whole screen; a resizable window means nothing there. */
    return SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN;
#else
    return SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL;
#endif
}

}

Window::Window(const char *title, int /*x*/, int /*y*/, int w, int h)
    :   m_window(SDL_CreateWindow(title, w, h, requestGlContext()))
{
    if (!m_window)
    {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Failed to create window: %s", SDL_GetError());
        return;
    }
}

Window::~Window()
{
}

}
