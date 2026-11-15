#include "host_window.h"

#include <lib/thread.h>
#include <lib/window.h>
#include <x86.h>

#include <SDL3/SDL.h>

/* The logic is the port's Windows' (src/lib/winapi/user32.cpp, gdi32.cpp) as the game ran it:
 * moved here, not changed.  Where that answered a constant (a stub of a function the port has no
 * use for), so does this, its assertion and trace kept. */
namespace nfs3hp
{
namespace host
{

static_assert(sizeof(Message) == sizeof(win32::MSG), "a message as the game keeps it");
static_assert(sizeof(WindowClassDesc) == 40, "WNDCLASSA");

x86::reg32 registerClass(App* app, const WindowClassDesc* description)
{
    win32::WindowClass* windowClass = new win32::WindowClass(description->windowProcedure);
    return app->allocateResource(windowClass);
}

x86::reg32 createWindow(App* app, x86::reg32 extendedStyle, x86::reg32 className, const char* title, x86::reg32 style,
                        int x, int y, int width, int height, x86::reg32 parent, x86::reg32 menu, x86::reg32 instance,
                        x86::reg32 parameter)
{
    NFS2_USE(extendedStyle);
    NFS2_USE(className);
    NFS2_USE(style);
    NFS2_USE(parent);
    NFS2_USE(menu);
    NFS2_USE(instance);
    NFS2_USE(parameter);
    win32::Window* window = new win32::Window(title, x, y, width, height);
    return app->allocateResource(window);
}

x86::reg32 destroyWindow(x86::reg32 window)
{
    win32::Window::postMessage(window, 0x0002, 0, 0);
    return 0;
}

std::int32_t defaultWindowProcedure(App* app, x86::reg32 window, x86::reg32 message, x86::reg32 wParam,
                                    x86::reg32 lParam)
{
    NFS2_USE(wParam);
    NFS2_USE(lParam);
    if (message == 0x0002)  // WM_DESTROY
        app->freeResource(window);
    return 0;
}

x86::reg32 showWindow(x86::reg32 window, int how)
{
    NFS2_USE(window);
    NFS2_USE(how);
    return 0;
}

x86::reg32 setWindowPosition(x86::reg32 window, x86::reg32 after, int x, int y, int width, int height,
                             x86::reg32 flags)
{
    NFS2_USE(window);
    NFS2_USE(after);
    NFS2_USE(x);
    NFS2_USE(y);
    NFS2_USE(width);
    NFS2_USE(height);
    NFS2_USE(flags);
    NFS2_ASSERT(false);
    return 0;
}

x86::reg32 clientRect(x86::reg32 window, Rect* rect)
{
    NFS2_USE(window);
    rect->top = 0;
    rect->left = 0;
    rect->right = 800;
    rect->bottom = 600;
    return 1;
}

x86::reg32 clientToScreen(x86::reg32 window, Point* point)
{
    NFS2_USE(window);
    NFS2_USE(point);
    return 1;
}

x86::reg32 adjustWindowRect(Rect* rect, x86::reg32 style, x86::reg32 menu)
{
    NFS2_USE(rect);
    NFS2_USE(style);
    NFS2_USE(menu);
    return 1;
}

std::int32_t windowLong(x86::reg32 window, int index)
{
    NFS2_USE(window);
    NFS2_USE(index);
    /* Always 0, for every index -- including GWL_STYLE and GWL_USERDATA, which
     * a menu could legitimately use to find its own state.  Rate limited in
     * case it sits in a poll loop. */
#ifdef NFS_TRACE_MSG
    static unsigned hits = 0;
    if (win32::msgTraceRateLimit(hits))
        NFS_MSG_TRACE("GetWindowLongA hWnd=%u nIndex=%d -> 0 (stub, hit %u)",
                      unsigned(window), index, hits);
#endif
    return 0;
}

x86::reg32 isIconic(x86::reg32 window)
{
    NFS2_USE(window);
    return 0;
}

x86::reg32 setForegroundWindow(x86::reg32 window)
{
    NFS2_USE(window);
    return 1;
}

x86::reg32 setActiveWindow(x86::reg32 window)
{
    NFS2_USE(window);
    return 0;
}

x86::reg32 findWindow(x86::reg32 className, x86::reg32 windowName)
{
    NFS2_USE(className);
    NFS2_USE(windowName);
    return 0;
}

x86::reg32 desktopWindow()
{
    NFS2_ASSERT(false);
    return x86::reg32(-1);
}

x86::reg32 windowDC(x86::reg32 window)
{
    NFS2_USE(window);
    NFS2_ASSERT(false);
    return x86::reg32(-1);
}

x86::reg32 beginPaint(x86::reg32 window, x86::reg32 paint)
{
    NFS2_USE(window);
    NFS2_USE(paint);
    /* The game imports BeginPaint/EndPaint, so it has a WM_PAINT path.  This
     * port never posts WM_PAINT and never validates a region, so if these fire
     * at all the repaint model is not the one assumed here. */
    NFS_MSG_TRACE("BeginPaint hWnd=%u (stub)", unsigned(window));
    NFS2_ASSERT(false);
    return x86::reg32(-1);
}

x86::reg32 endPaint(x86::reg32 window, x86::reg32 paint)
{
    NFS2_USE(window);
    NFS2_USE(paint);
    NFS_MSG_TRACE("EndPaint hWnd=%u (stub)", unsigned(window));
    NFS2_ASSERT(false);
    return 0;
}

x86::reg32 loadIcon(x86::reg32 instance, x86::reg32 name)
{
    NFS2_USE(instance);
    NFS2_USE(name);
    return 0;
}

x86::reg32 getMessage(App* app, x86::CPU& cpu, Message* message, x86::reg32 window, x86::reg32 filterMin,
                      x86::reg32 filterMax)
{
    NFS2_USE(window);
    win32::MSG* msg = reinterpret_cast<win32::MSG*>(message);
    win32::Window* owner = msg->hWindow ? dynamic_cast<win32::Window*>(app->getResource(msg->hWindow)) : nullptr;
    app->unlockContext(cpu);
    const x86::reg32 result = win32::Window::getMessage(cpu, msg, owner, filterMin, filterMax);
    app->lockContext(cpu);
#ifdef NFS_TRACE_MSG
    SDL_Log("[MSG] GetMessage -> %d  msg=0x%04x wParam=0x%08x lParam=0x%08x hWnd=%u thread=%u",
            int(result), unsigned(msg->message), unsigned(msg->wParam),
            unsigned(msg->lParam), unsigned(msg->hWindow), unsigned(win32::Thread::currentThreadId()));
#endif
    return result;
}

x86::reg32 translateMessage(const Message* message)
{
    NFS2_USE(message);
    return 0;
}

std::int32_t dispatchMessage(App* app, x86::CPU& cpu, const Message* message)
{
    const x86::reg32 handler = win32::Window::getMessageHandler();
#ifdef NFS_TRACE_MSG
    /* Enter/exit around the guest window procedure.  A message that is
     * delivered but never dispatched, and a dispatch that never returns, are
     * two different bugs and only this pair tells them apart; frames_inside
     * says whether the repaint happens from within message handling. */
    const Uint64 dispatchStart = SDL_GetTicks();
    const unsigned framesBefore = win32::g_traceFrames;
    NFS_MSG_TRACE("DispatchMsgA  -> wndproc=0x%08x msg=0x%04x wParam=0x%08x lParam=0x%08x hWnd=%u",
                  unsigned(handler), unsigned(message->message), unsigned(message->wParam),
                  unsigned(message->lParam), unsigned(message->window));
#endif
    cpu.esp -= 20;
    app->getMemory<x86::reg32>(cpu.esp + 16) = message->lParam;
    app->getMemory<x86::reg32>(cpu.esp + 12) = message->wParam;
    app->getMemory<x86::reg32>(cpu.esp + 8) = message->message;
    app->getMemory<x86::reg32>(cpu.esp + 4) = message->window;
    app->getMemory<x86::reg32>(cpu.esp + 0) = handler;
    app->dynamic_call(handler, cpu);
#ifdef NFS_TRACE_MSG
    NFS_MSG_TRACE("DispatchMsgA  <- msg=0x%04x eax=0x%08x took=%ums frames_inside=%u",
                  unsigned(message->message), unsigned(cpu.eax),
                  unsigned(SDL_GetTicks() - dispatchStart),
                  win32::g_traceFrames - framesBefore);
#endif
    return x86::sreg32(cpu.eax);
}

x86::reg32 postMessage(x86::reg32 window, x86::reg32 message, x86::reg32 wParam, x86::reg32 lParam)
{
    return win32::Window::postMessage(window, message, wParam, lParam);
}

x86::reg32 sendNotifyMessage(x86::reg32 window, x86::reg32 message, x86::reg32 wParam, x86::reg32 lParam)
{
    NFS2_USE(window);
    NFS2_USE(message);
    NFS2_USE(wParam);
    NFS2_USE(lParam);
    /* Same as PostThreadMessageA: silently dropped, and unlike a real
     * SendNotifyMessage the window procedure is never entered. */
    NFS_MSG_TRACE("SendNotifyMessageA DROPPED hWnd=%u msg=0x%04x wParam=0x%08x lParam=0x%08x",
                  unsigned(window), unsigned(message), unsigned(wParam), unsigned(lParam));
    NFS2_ASSERT(false);
    return 0;
}

x86::reg32 postThreadMessage(x86::reg32 thread, x86::reg32 message, x86::reg32 wParam, x86::reg32 lParam)
{
    NFS2_USE(thread);
    NFS2_USE(message);
    NFS2_USE(wParam);
    NFS2_USE(lParam);
    /* Dropped on the floor: nothing is queued and the caller is told it
     * failed.  This is how a worker or a timer callback would wake one guest
     * thread's pump, so if it ever fires on the settings screens it is a
     * direct candidate for "no repaint without input" -- hence the trace and
     * not only the assert. */
    NFS_MSG_TRACE("PostThreadMessageA DROPPED tid=%u msg=0x%04x wParam=0x%08x lParam=0x%08x",
                  unsigned(thread), unsigned(message), unsigned(wParam), unsigned(lParam));
    NFS2_ASSERT(false);
    return 0;
}

void postQuitMessage(App* app, x86::CPU& cpu, int code)
{
    /* Said, with the game's code that asked for it: a game that closes by
     * itself leaves nothing else behind to tell why. */
    SDL_Log("[EXIT] PostQuitMessage(%d) from 0x%x", code,
            unsigned(app->getMemory<x86::reg32>(cpu.esp)));
    win32::Window::postMessage(0, 0x0012, 0, 0);
}

x86::reg32 setWindowsHook(int kind, x86::reg32 procedure, x86::reg32 module, x86::reg32 thread)
{
    NFS2_USE(procedure);
    NFS2_USE(module);
    NFS2_USE(thread);
    /* kind 2 is WH_KEYBOARD.  The hook is accepted with a null handle and
     * the procedure is never called back, so whatever the game routes through
     * it never happens -- a candidate for lost menu input, not for the missing
     * idle repaint. */
    NFS_MSG_TRACE("SetWindowsHookExA idHook=%d lpfn=0x%08x thread=%u -> 0 (never called back)",
                  kind, unsigned(procedure), unsigned(thread));
    NFS2_ASSERT(kind == 2);
    return 0;
}

x86::reg32 unhookWindowsHook(x86::reg32 hook)
{
    NFS2_USE(hook);
    NFS_MSG_TRACE("UnhookWindowsHookEx hhk=%u", unsigned(hook));
    NFS2_ASSERT(false);
    return 0;
}

std::int32_t callNextHook(x86::reg32 hook, int code, x86::reg32 wParam, x86::reg32 lParam)
{
    NFS2_USE(hook);
    NFS2_USE(code);
    NFS2_USE(wParam);
    NFS2_USE(lParam);
    /* Only reachable from inside a hook procedure, i.e. only if the game runs
     * one -- which it cannot here, since setWindowsHook never calls back.
     * If this ever appears in a trace, that assumption is wrong. */
    NFS_MSG_TRACE("CallNextHookEx nCode=%d wParam=0x%08x lParam=0x%08x", code,
                  unsigned(wParam), unsigned(lParam));
    NFS2_ASSERT(false);
    return 0;
}

int showCursor(x86::reg32 show)
{
    NFS2_USE(show);
    return 0;
}

x86::reg32 setCursor(x86::reg32 cursor)
{
    NFS2_USE(cursor);
    return 0;
}

x86::reg32 setCursorPosition(int x, int y)
{
    NFS2_USE(x);
    NFS2_USE(y);
    NFS2_ASSERT(false);
    return 0;
}

x86::reg32 swapMouseButton(x86::reg32 swap)
{
    NFS2_USE(swap);
    return 0;
}

std::int16_t asyncKeyState(int key)
{
    /* The high bit means "currently down", as GetAsyncKeyState's; a screen
     * driven by polling this (the graphics options screen) sees input by it. */
#ifdef NFS_TRACE_MSG
    static unsigned hits = 0;
    if (win32::msgTraceRateLimit(hits))
        NFS_MSG_TRACE("GetAsyncKeyState vKey=0x%02x down=%d (hit %u)",
                      unsigned(key), int(win32::Window::isKeyDown(key)), hits);
#endif
    return win32::Window::isKeyDown(key) ? std::int16_t(-32768) : std::int16_t(0);
}

std::int16_t keyState(int key)
{
    /* As asyncKeyState.  The toggle bit (low byte, for VK_CAPITAL/NUMLOCK/
     * SCROLL) is deliberately left at 0: the port has no concept of lock-key
     * toggle state, and nothing in nfs3hp reads it. */
#ifdef NFS_TRACE_MSG
    static unsigned hits = 0;
    if (win32::msgTraceRateLimit(hits))
        NFS_MSG_TRACE("GetKeyState vKey=0x%02x down=%d (hit %u)",
                      unsigned(key), int(win32::Window::isKeyDown(key)), hits);
#endif
    return win32::Window::isKeyDown(key) ? std::int16_t(-32768) : std::int16_t(0);
}

int keyboardType(int which)
{
    switch (which)
    {
    case 0:
    case 1:
    case 2:
        return 0;
    default:
        NFS2_ASSERT(false);
        return 0;
    }
}

void keyboardEvent(std::uint8_t key, std::uint8_t scan, x86::reg32 flags, x86::reg32 extra)
{
    NFS2_USE(key);
    NFS2_USE(scan);
    NFS2_USE(flags);
    NFS2_USE(extra);
}

int systemMetrics(int index)
{
    switch (index)
    {
    case 0:   // SM_CXSCREEN
        return 800;
    case 1:   // SM_CYSCREEN
        return 600;
    case 63:  // SM_NETWORK: bit 0, a network is present; without it the game offers no TCP/IP (0x4f7830)
        return 1;
    default:
        NFS2_ASSERT(false);
        return 0;
    }
}

x86::reg32 systemParameters(x86::reg32 action, x86::reg32 parameter, x86::reg32* value, x86::reg32 profile)
{
    NFS2_USE(parameter);
    NFS2_USE(profile);
    switch (action)
    {
    case 0x10:  // SPI_GETSCREENSAVEACTIVE
    case 0x54:  // SPI_GETPOWEROFFACTIVE
        *value = 0;
        break;
    default:
        NFS2_ASSERT(false);
    }
    return 1;
}

int deviceCaps(x86::reg32 dc, int index)
{
    NFS2_USE(dc);
    NFS2_USE(index);
    NFS2_ASSERT(false);
    return -1;
}

x86::reg32 stockObject(int which)
{
    NFS2_USE(which);
    return 0xffffffff;
}

}  // namespace host
}  // namespace nfs3hp
