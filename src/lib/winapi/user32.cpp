#include <winapi/user32.h>
#include <x86.h>
#include <lib/window.h>
#include <lib/thread.h>
#include <cctype>
#include <SDL3/SDL.h>

namespace win32 { namespace user32
{

BOOL AdjustWindowRect(WinApplication* app, x86::CPU& cpu,
                      LPRECT lpRect, DWORD dwStyle, BOOL bMenu)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(lpRect);
    NFS2_USE(dwStyle);
    NFS2_USE(bMenu);
    return 1;
}

HDC BeginPaint(WinApplication* app, x86::CPU& cpu,
               HWND hWnd, LPPAINTSTRUCT lpPaint)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hWnd);
    NFS2_USE(lpPaint);
    /* The game imports BeginPaint/EndPaint, so it has a WM_PAINT path.  This
     * port never posts WM_PAINT and never validates a region, so if these fire
     * at all the repaint model is not the one assumed here. */
    NFS_MSG_TRACE("BeginPaint hWnd=%u (stub)", unsigned(hWnd));
    NFS2_ASSERT(false);
    return x86::reg32(-1);
}

LRESULT CallNextHookEx(WinApplication* app, x86::CPU& cpu,
                       HHOOK  hhk, int nCode, WPARAM wParam, LPARAM lParam)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hhk);
    NFS2_USE(nCode);
    NFS2_USE(wParam);
    NFS2_USE(lParam);
    /* Only reachable from inside a hook procedure, i.e. only if the game runs
     * one -- which it cannot here, since SetWindowsHookExA never calls back.
     * If this ever appears in a trace, that assumption is wrong. */
    NFS_MSG_TRACE("CallNextHookEx nCode=%d wParam=0x%08x lParam=0x%08x", nCode,
                  unsigned(wParam), unsigned(lParam));
    NFS2_ASSERT(false);
    return 0;
}

x86::reg32 CharUpperA(WinApplication* app, x86::CPU& cpu,
                      LPSTR lpsz)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    for (char* c = lpsz; *c; ++c)
        *c = char(toupper(*c));
    return app->getMemory<x86::reg32>(cpu.esp + 4);
}

DWORD CharUpperBuffA(WinApplication* app, x86::CPU& cpu,
                     LPSTR lpsz, DWORD cchLength)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    for (x86::reg32 i = 0; i < cchLength; ++i)
    {
        lpsz[i] = char(toupper(lpsz[i]));
    }
    return cchLength;
}

BOOL ClientToScreen(WinApplication* app, x86::CPU& cpu,
                    HWND hWnd, LPPOINT lpPoint)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hWnd);
    NFS2_USE(lpPoint);
    return 1;
}

HWND CreateWindowExA(WinApplication* app, x86::CPU& cpu,
                     DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle,
                     int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu,
                     HINSTANCE hInstance, LPVOID lpParam)
{
    NFS2_USE(cpu);
    NFS2_USE(dwExStyle);
    NFS2_USE(lpClassName);
    NFS2_USE(dwStyle);
    NFS2_USE(hWndParent);
    NFS2_USE(hMenu);
    NFS2_USE(hInstance);
    NFS2_USE(lpParam);
    win32::Window* window = new win32::Window(lpWindowName, X, Y, nWidth, nHeight);
    return app->allocateResource(window);
}

HDDEDATA DdeClientTransaction(WinApplication* app, x86::CPU& cpu,
                              LPBYTE pData, DWORD cbData, HCONV hConv, HSZ hszItem,
                              UINT wFmt, UINT wType, DWORD dwTimeout, LPDWORD pdwResult)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(pData);
    NFS2_USE(cbData);
    NFS2_USE(hConv);
    NFS2_USE(hszItem);
    NFS2_USE(wFmt);
    NFS2_USE(wType);
    NFS2_USE(dwTimeout);
    NFS2_USE(pdwResult);
    NFS2_ASSERT(false);
    return -1;
}

HCONV DdeConnect(WinApplication* app, x86::CPU& cpu, DWORD idInst,
                 HSZ hszService, HSZ hszTopic, PCONVCONTEXT pCC)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(idInst);
    NFS2_USE(hszService);
    NFS2_USE(hszTopic);
    NFS2_USE(pCC);
    NFS2_ASSERT(false);
    return -1;
}

HSZ DdeCreateStringHandleA(WinApplication* app, x86::CPU& cpu,
                           DWORD idInst, LPCSTR psz, int iCodePage)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(idInst);
    NFS2_USE(psz);
    NFS2_USE(iCodePage);
    NFS2_ASSERT(false);
    return -1;
}

UINT DdeGetLastError(WinApplication* app, x86::CPU& cpu, DWORD idInst)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(idInst);
    NFS2_ASSERT(false);
    return -1;
}

UINT DdeInitializeA(WinApplication* app, x86::CPU& cpu, LPDWORD pidInst,
                    PFNCALLBACK pfnCallback, DWORD afCmd, DWORD ulRes)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(pidInst);
    NFS2_USE(pfnCallback);
    NFS2_USE(afCmd);
    NFS2_USE(ulRes);
    NFS2_ASSERT(false);
    return -1;
}

DWORD DdeQueryStringA(WinApplication* app, x86::CPU& cpu, DWORD idInst,
                      HSZ hsz, LPSTR psz, DWORD cchMax, int iCodePage)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(idInst);
    NFS2_USE(hsz);
    NFS2_USE(psz);
    NFS2_USE(cchMax);
    NFS2_USE(iCodePage);
    NFS2_ASSERT(false);
    return -1;
}

BOOL DdeUninitialize(WinApplication* app, x86::CPU& cpu, DWORD idInst)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(idInst);
    NFS2_ASSERT(false);
    return 0;
}

LRESULT DefWindowProcA(WinApplication* app, x86::CPU& cpu,
                       HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    NFS2_USE(cpu);
    NFS2_USE(wParam);
    NFS2_USE(lParam);
    if (Msg == 0x0002)
    {
        app->freeResource(hWnd);
    }
    else
    {
        //NFS2_ASSERT(false);
    }
    return 0;
}

BOOL DestroyWindow(WinApplication* app, x86::CPU& cpu,
                   HWND hWnd)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hWnd);
    Window::postMessage(hWnd, 0x0002, 0, 0);
    return 0;
}

LRESULT DispatchMessageA(WinApplication* app, x86::CPU& cpu,
                         const MSG *lpMsg)
{
    x86::reg32 handler = win32::Window::getMessageHandler();
#ifdef NFS_TRACE_MSG
    /* Enter/exit around the guest window procedure.  A message that is
     * delivered but never dispatched, and a dispatch that never returns, are
     * two different bugs and only this pair tells them apart; frames_inside
     * says whether the repaint happens from within message handling. */
    const Uint64 dispatchStart = SDL_GetTicks();
    const unsigned framesBefore = win32::g_traceFrames;
    NFS_MSG_TRACE("DispatchMsgA  -> wndproc=0x%08x msg=0x%04x wParam=0x%08x lParam=0x%08x hWnd=%u",
                  unsigned(handler), unsigned(lpMsg->message), unsigned(lpMsg->wParam),
                  unsigned(lpMsg->lParam), unsigned(lpMsg->hWindow));
#endif
    cpu.esp -= 20;
    app->getMemory<x86::reg32>(cpu.esp + 16) = lpMsg->lParam;
    app->getMemory<x86::reg32>(cpu.esp + 12) = lpMsg->wParam;
    app->getMemory<x86::reg32>(cpu.esp + 8) = lpMsg->message;
    app->getMemory<x86::reg32>(cpu.esp + 4) = lpMsg->hWindow;
    app->getMemory<x86::reg32>(cpu.esp + 0) = handler;
    app->dynamic_call(handler, cpu);
#ifdef NFS_TRACE_MSG
    NFS_MSG_TRACE("DispatchMsgA  <- msg=0x%04x eax=0x%08x took=%ums frames_inside=%u",
                  unsigned(lpMsg->message), unsigned(cpu.eax),
                  unsigned(SDL_GetTicks() - dispatchStart),
                  win32::g_traceFrames - framesBefore);
#endif
    return x86::sreg32(cpu.eax);
}

BOOL EndPaint(WinApplication* app, x86::CPU& cpu,
              HWND hWnd, const PAINTSTRUCT *lpPaint)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hWnd);
    NFS2_USE(lpPaint);
    NFS_MSG_TRACE("EndPaint hWnd=%u (stub)", unsigned(hWnd));
    NFS2_ASSERT(false);
    return 0;
}

HWND FindWindowA(WinApplication* app, x86::CPU& cpu,
                 LPCSTR lpClassName, LPCSTR lpWindowName)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(lpClassName);
    NFS2_USE(lpWindowName);
    //NFS2_ASSERT(false);
    return 0;
}

SHORT GetAsyncKeyState(WinApplication* app, x86::CPU& cpu,
                       int vKey)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    /* Upstream hard-returned 0 here -- "never pressed" -- which is why a
     * screen driven by polling this (rather than the WM_KEYDOWN message
     * queue, e.g. the graphics options screen) never saw any input and only
     * ever redrew once per keypress, via the unrelated message-queue path.
     * The high bit means "currently down", matching real GetAsyncKeyState. */
#ifdef NFS_TRACE_MSG
    static unsigned hits = 0;
    if (win32::msgTraceRateLimit(hits))
        NFS_MSG_TRACE("GetAsyncKeyState vKey=0x%02x down=%d (hit %u)",
                      unsigned(vKey), int(Window::isKeyDown(vKey)), hits);
#endif
    return Window::isKeyDown(vKey) ? SHORT(-32768) : SHORT(0);
}

BOOL GetClientRect(WinApplication* app, x86::CPU& cpu,
                   HWND hWnd, LPRECT lpRect)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hWnd);
    lpRect->top = 0;
    lpRect->left = 0;
    lpRect->right = 800;
    lpRect->bottom = 600;
    return 1;
}

HWND GetDesktopWindow(WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_ASSERT(false);
    return -1;
}

SHORT GetKeyState(WinApplication* app, x86::CPU& cpu,
                  int nVirtKey)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    /* Same fix as GetAsyncKeyState above.  The toggle bit (low byte, for
     * VK_CAPITAL/NUMLOCK/SCROLL) is deliberately left at 0: the port has no
     * concept of lock-key toggle state, and nothing in nfs3hp reads it. */
#ifdef NFS_TRACE_MSG
    /* Rate limited: a screen polling this in a loop would fill the log by
     * itself.  What matters is whether it is polled at all on the stalled
     * screens -- polling would mean the screen does not need the message queue
     * to notice input, but does need something to keep calling it. */
    static unsigned hits = 0;
    if (win32::msgTraceRateLimit(hits))
        NFS_MSG_TRACE("GetKeyState vKey=0x%02x down=%d (hit %u)",
                      unsigned(nVirtKey), int(Window::isKeyDown(nVirtKey)), hits);
#endif
    return Window::isKeyDown(nVirtKey) ? SHORT(-32768) : SHORT(0);
}

int GetKeyboardType(WinApplication* app, x86::CPU& cpu,
                    int nTypeFlag)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    switch(nTypeFlag)
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

BOOL GetMessageA(WinApplication* app, x86::CPU& cpu,
                 LPMSG lpMsg, HWND  hWnd, UINT  wMsgFilterMin, UINT  wMsgFilterMax)
{
    NFS2_USE(cpu);
    NFS2_USE(hWnd);
    win32::Window* window = lpMsg->hWindow ? dynamic_cast<win32::Window*>(app->getResource(lpMsg->hWindow)) : nullptr;
    app->unlockContext(cpu);
    BOOL result = win32::Window::getMessage(cpu, lpMsg, window, wMsgFilterMin, wMsgFilterMax);
    app->lockContext(cpu);
#ifdef NFS_TRACE_MSG
    SDL_Log("[MSG] GetMessage -> %d  msg=0x%04x wParam=0x%08x lParam=0x%08x hWnd=%u thread=%u",
            int(result), unsigned(lpMsg->message), unsigned(lpMsg->wParam),
            unsigned(lpMsg->lParam), unsigned(lpMsg->hWindow), unsigned(win32::Thread::currentThreadId()));
#endif
    return result;
}

int GetSystemMetrics(WinApplication* app, x86::CPU& cpu,
                     int nIndex)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    switch(nIndex)
    {
    case 0: //SM_CXSCREEN
        return 800;
    case 1: //SM_CYSCREEN
        return 600;
    default:
        NFS2_ASSERT(false);
        return 0;
    }
}

HDC GetWindowDC(WinApplication* app, x86::CPU& cpu,
                HWND hWnd)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hWnd);
    NFS2_ASSERT(false);
    return -1;
}

LONG GetWindowLongA(WinApplication* app, x86::CPU& cpu,
                    HWND hWnd, int nIndex)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hWnd);
    NFS2_USE(nIndex);
    /* Always 0, for every index -- including GWL_STYLE and GWL_USERDATA, which
     * a menu could legitimately use to find its own state.  Rate limited in
     * case it sits in a poll loop. */
#ifdef NFS_TRACE_MSG
    static unsigned hits = 0;
    if (win32::msgTraceRateLimit(hits))
        NFS_MSG_TRACE("GetWindowLongA hWnd=%u nIndex=%d -> 0 (stub, hit %u)",
                      unsigned(hWnd), nIndex, hits);
#endif
    return 0;
}

BOOL IsIconic(WinApplication* app, x86::CPU& cpu,
              HWND hWnd)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hWnd);
    return 0;
}

HICON LoadIconA(WinApplication* app, x86::CPU& cpu,
                HINSTANCE hInstance, LPCSTR lpIconName)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hInstance);
    NFS2_USE(lpIconName);
    return 0;
}

int MessageBoxA(WinApplication* app, x86::CPU& cpu,
                HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hWnd);
    Uint32 type = 0;
    if (uType & 0x0001) type |= SDL_MESSAGEBOX_ERROR;
    if (uType & 0x0002) type |= SDL_MESSAGEBOX_WARNING;
    if (uType & 0x0000) type |= SDL_MESSAGEBOX_INFORMATION;
    SDL_ShowSimpleMessageBox(type, lpCaption, lpText, nullptr);
    return 0;
}

BOOL OffsetRect(WinApplication* app, x86::CPU& cpu,
                LPRECT lprc, int dx, int dy)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    lprc->left += dx;
    lprc->right += dx;
    lprc->top += dy;
    lprc->bottom += dy;
    return 1;
}

BOOL PostMessageA(WinApplication* app, x86::CPU& cpu,
                  HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    return win32::Window::postMessage(hWnd, Msg, wParam, lParam);
}

void PostQuitMessage(WinApplication* app, x86::CPU& cpu,
                     int nExitCode)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(nExitCode);
    win32::Window::postMessage(0, 0x0012, 0, 0);
}

BOOL PostThreadMessageA(WinApplication* app, x86::CPU& cpu,
                        DWORD idThread, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(idThread);
    NFS2_USE(Msg);
    NFS2_USE(wParam);
    NFS2_USE(lParam);
    /* Dropped on the floor: nothing is queued and the caller is told it
     * failed.  This is how a worker or a timer callback would wake one guest
     * thread's pump, so if it ever fires on the settings screens it is a
     * direct candidate for "no repaint without input" -- hence the trace and
     * not only the assert. */
    NFS_MSG_TRACE("PostThreadMessageA DROPPED tid=%u msg=0x%04x wParam=0x%08x lParam=0x%08x",
                  unsigned(idThread), unsigned(Msg), unsigned(wParam), unsigned(lParam));
    NFS2_ASSERT(false);
    return 0;
}

ATOM RegisterClassA(WinApplication* app, x86::CPU& cpu,
                    const WNDCLASSA *lpWndClass)
{
    NFS2_USE(cpu);
    win32::WindowClass* wClass = new win32::WindowClass(lpWndClass->wndProc);
    return app->allocateResource(wClass);
}

BOOL SendNotifyMessageA(WinApplication* app, x86::CPU& cpu,
                        HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hWnd);
    NFS2_USE(Msg);
    NFS2_USE(wParam);
    NFS2_USE(lParam);
    /* Same as PostThreadMessageA: silently dropped, and unlike a real
     * SendNotifyMessage the window procedure is never entered. */
    NFS_MSG_TRACE("SendNotifyMessageA DROPPED hWnd=%u msg=0x%04x wParam=0x%08x lParam=0x%08x",
                  unsigned(hWnd), unsigned(Msg), unsigned(wParam), unsigned(lParam));
    NFS2_ASSERT(false);
    return 0;
}

HWND SetActiveWindow(WinApplication* app, x86::CPU& cpu,
                     HWND hWnd)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hWnd);
    return 0;
}

HCURSOR SetCursor(WinApplication* app, x86::CPU& cpu,
                  HCURSOR hCursor)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hCursor);
    return 0;
}

BOOL SetCursorPos(WinApplication* app, x86::CPU& cpu,
                  int X, int Y)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(X);
    NFS2_USE(Y);
    NFS2_ASSERT(false);
    return 0;
}

BOOL SetForegroundWindow(WinApplication* app, x86::CPU& cpu,
                         HWND hWnd)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hWnd);
    return 1;
}

BOOL SetWindowPos(WinApplication* app, x86::CPU& cpu,
                  HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hWnd);
    NFS2_USE(hWndInsertAfter);
    NFS2_USE(X);
    NFS2_USE(Y);
    NFS2_USE(cx);
    NFS2_USE(cy);
    NFS2_USE(uFlags);
    NFS2_ASSERT(false);
    return 0;
}

HHOOK SetWindowsHookExA(WinApplication* app, x86::CPU& cpu,
                        int idHook, x86::reg32 lpfn, HINSTANCE hmod, DWORD dwThreadId)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(lpfn);
    NFS2_USE(hmod);
    NFS2_USE(dwThreadId);
    /* idHook 2 is WH_KEYBOARD.  The hook is accepted with a null handle and
     * lpfn is never called back, so whatever the game routes through it never
     * happens -- a candidate for lost menu input, not for the missing idle
     * repaint.  lpfn is a guest address: it is what lets the generated code
     * for that hook procedure be found. */
    NFS_MSG_TRACE("SetWindowsHookExA idHook=%d lpfn=0x%08x thread=%u -> 0 (never called back)",
                  idHook, unsigned(lpfn), unsigned(dwThreadId));
    NFS2_ASSERT(idHook == 2);
    return 0;
}

int ShowCursor(WinApplication* app, x86::CPU& cpu,
               BOOL bShow)
{
    // TODO?
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(bShow);
    return 0;
}

BOOL ShowWindow(WinApplication* app, x86::CPU& cpu,
                HWND hWnd, int nCmdShow)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hWnd);
    NFS2_USE(nCmdShow);
    return 0;
}

BOOL SwapMouseButton(WinApplication* app, x86::CPU& cpu, BOOL fSwap)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(fSwap);
    return 0;
}

BOOL SystemParametersInfoA(WinApplication* app, x86::CPU& cpu,
                           UINT uiAction, UINT uiParam, PVOID pvParam, UINT  fWinIni)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(uiParam);
    NFS2_USE(fWinIni);
    switch(uiAction)
    {
    case 0x10: //SPI_GETSCREENSAVEACTIVE
    case 0x54: //SPI_GETPOWEROFFACTIVE
        *static_cast<x86::reg32*>(pvParam) = 0;
        break;
    default:
        NFS2_ASSERT(false);
    }
    return 1;
}

BOOL TranslateMessage(WinApplication* app, x86::CPU& cpu,
                      const MSG *lpMsg)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(lpMsg);
    return 0;
}

BOOL UnhookWindowsHookEx(WinApplication* app, x86::CPU& cpu,
                         HHOOK hhk)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hhk);
    NFS_MSG_TRACE("UnhookWindowsHookEx hhk=%u", unsigned(hhk));
    NFS2_ASSERT(false);
    return 0;
}

void keybd_event(WinApplication* app, x86::CPU& cpu,
                 BYTE bVk, BYTE bScan, DWORD dwFlags, ULONG_PTR dwExtraInfo)
{
    //NFS2_ASSERT(false);
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(bVk);
    NFS2_USE(bScan);
    NFS2_USE(dwFlags);
    NFS2_USE(dwExtraInfo);
}

void wsprintfA(WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_ASSERT(false);
}

}}
