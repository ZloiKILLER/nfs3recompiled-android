#ifndef LIB_WINDOW_H_
#define LIB_WINDOW_H_

#include <lib/winapp.h>
#include <winapi/types.h>
#include <SDL3/SDL.h>

namespace win32
{

extern Uint32 g_userEvent;
extern Uint32 g_userEvent1;
extern Uint32 g_userEvent2;
extern Uint32 g_wmCharEvent;

/* ---------------------------------------------------------------------------
 * Diagnostic message-pump trace  (cmake -DNFS_TRACE_MSG=ON)
 *
 * getMessage() below blocks in SDL_WaitEvent, which faithfully reproduces the
 * original: nfs3.exe imports GetMessageA and neither PeekMessageA nor
 * SetTimer, so its own pump blocked too.  Anything that repaints without user
 * input therefore has to post into the queue from somewhere else -- a winmm
 * timer callback, another guest thread, the window procedure itself.  These
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

void msgTrace(SDL_PRINTF_FORMAT_STRING const char* fmt, ...) SDL_PRINTF_VARARG_FUNC(1);

/* Keeps a high-frequency site (GetKeyState in a poll loop, a 1 ms timer) from
 * changing the very timings being measured: the first hits verbatim, then
 * every 64th, so "still happening" stays visible at a bounded cost. */
bool msgTraceRateLimit(unsigned& hits);

# define NFS_MSG_TRACE(...)     ::win32::msgTrace(__VA_ARGS__)
# define NFS_MSG_FRAME()        (++::win32::g_traceFrames)
#else
# define NFS_MSG_TRACE(...)     ((void)0)
# define NFS_MSG_FRAME()        ((void)0)
#endif

class WindowClass : public GenericResource
{
    friend class Window;
public:
    WindowClass(x86::reg32 windowProc);
    ~WindowClass();

private:
    x86::reg32 m_wndProc;
};

class Window : public GenericResource
{
    friend class Renderer;
public:
    Window(const char* title, int x, int y, int w, int h);
    ~Window();

    static x86::reg32 getMessage(const x86::CPU& cpu, MSG* result, Window* window, x86::reg32 filterMin, x86::reg32 msgMax);
    static x86::reg32 postMessage(x86::reg32 hWnd, x86::reg32 message, x86::reg32 wParam, x86::reg32 lParam);
    static x86::reg32 getMessageHandler();

    /* Live keyboard state, indexed by the same Win32 virtual-key code that
     * getMessage() already computes into MSG::wParam for WM_KEYDOWN/KEYUP.
     * Backs GetAsyncKeyState/GetKeyState (winapi/user32.cpp), which upstream
     * hard-returned 0 -- always "not pressed" -- regardless of real input. */
    static bool isKeyDown(int vkCode);

private:
    /* The real pump.  getMessage() is a thin wrapper that times the blocking
     * wait around it and logs what came out (NFS_TRACE_MSG). */
    static x86::reg32 getMessageImpl(const x86::CPU& cpu, MSG* result, Window* window, x86::reg32 filterMin, x86::reg32 msgMax);

    SDL_Window*     m_window;
};

}

#endif
