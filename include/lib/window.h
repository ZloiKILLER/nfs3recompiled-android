#ifndef LIB_WINDOW_H_
#define LIB_WINDOW_H_

#include <lib/app.h>
#include <SDL3/SDL.h>

namespace nfs
{

/* The port's own SDL events (src/nfs3hp/os_window.cpp): a message the game
 * posts, the port's word to stop or to let a window go, a thread woken to
 * stop, a character a key gave. */
extern Uint32 g_gameMessageEvent;
extern Uint32 g_portEvent;
extern Uint32 g_threadStopEvent;
extern Uint32 g_characterEvent;

/* ---------------------------------------------------------------------------
 * Diagnostic event trace  (cmake -DNFS_TRACE_MSG=ON)
 *
 * The window's wait (src/nfs3hp/os_window.cpp) blocks in SDL_WaitEvent, as the
 * original's loop did: nfs3.exe imports GetMessageA and neither PeekMessageA nor
 * SetTimer, so its own loop blocked too.  Anything that repaints without user
 * input therefore has to post into the queue from somewhere else -- a game
 * timer's callback, another guest thread, the window procedure itself.  These
 * hooks record who posts, who wakes, what the window procedure does with the
 * message and whether a presented frame actually followed, so the "one frame
 * per keypress" behaviour of the blist settings screens can be attributed
 * rather than guessed at.  Every call site is a macro that compiles to nothing
 * when the option is off; note the define is PRIVATE to nfs_core, so nothing
 * here may change a type's layout.
 * ------------------------------------------------------------------------ */
#ifdef NFS_TRACE_MSG
/* Presented frames, incremented wherever a present actually reaches the
 * screen.  Every trace line carries it, which is what ties "the game was told
 * about the keypress" to "a new frame came out". */
extern unsigned g_traceFrames;

void eventTrace(SDL_PRINTF_FORMAT_STRING const char* fmt, ...) SDL_PRINTF_VARARG_FUNC(1);

/* Keeps a high-frequency site (GetKeyState in a poll loop, a 1 ms timer) from
 * changing the very timings being measured: the first hits verbatim, then
 * every 64th, so "still happening" stays visible at a bounded cost. */
bool eventTraceRateLimit(unsigned& hits);

# define NFS_MSG_TRACE(...)     ::nfs::eventTrace(__VA_ARGS__)
# define NFS_MSG_FRAME()        (++::nfs::g_traceFrames)
#else
# define NFS_MSG_TRACE(...)     ((void)0)
# define NFS_MSG_FRAME()        ((void)0)
#endif

/* The game's window: SDL's, with a GL context's pixel format.  What reaches it is the port's
 * events (src/nfs3hp/os_window.cpp), on the user events above. */
class Window : public GenericResource
{
    friend class Renderer;
public:
    Window(const char* title, int x, int y, int w, int h);
    ~Window();

private:
    SDL_Window*     m_window;
};

}

#endif
