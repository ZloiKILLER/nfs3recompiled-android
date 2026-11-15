#ifndef NFS3HP_HOST_WINDOW_H
#define NFS3HP_HOST_WINDOW_H

#include <lib/winapp.h>

#include <cstdint>

/* The port's window, its messages, its cursor and keys and what it says of the screen, for the
 * game's calls of user32 and gdi32 (plan step 6): host code on src/lib's Window and SDL.  Each
 * takes the words the game's code pushes for the import it stands for, in its order (a pointer
 * one as a pointer into the game's memory), so a call site hands them as they are
 * (guest::windows<kSlot, &host::fn>, native_windows.h), and gives back what the game reads in eax:
 * the same values the port's Windows gave.  The window procedure is fed by the message pump
 * (getMessage, dispatchMessage) as before. */
namespace nfs3hp
{
namespace host
{

using App = win32::WinApplication;

struct Rect
{
    std::int32_t left;
    std::int32_t top;
    std::int32_t right;
    std::int32_t bottom;
};

struct Point
{
    std::int32_t x;
    std::int32_t y;
};

/* A message as the game keeps it (MSG). */
struct Message
{
    x86::reg32 window;
    x86::reg32 message;
    x86::reg32 wParam;
    x86::reg32 lParam;
    x86::reg32 time;
    Point point;
    x86::reg32 reserved;
};

/* A window class as the game describes it (WNDCLASSA). */
struct WindowClassDesc
{
    x86::reg32 style;
    x86::reg32 windowProcedure;
    std::int32_t classExtra;
    std::int32_t windowExtra;
    x86::reg32 instance;
    x86::reg32 icon;
    x86::reg32 cursor;
    x86::reg32 background;
    x86::reg32 menuName;
    x86::reg32 className;
};

// The window and its messages (user32).
x86::reg32 registerClass(App* app, const WindowClassDesc* description);
x86::reg32 createWindow(App* app, x86::reg32 extendedStyle, x86::reg32 className, const char* title, x86::reg32 style,
                        int x, int y, int width, int height, x86::reg32 parent, x86::reg32 menu, x86::reg32 instance,
                        x86::reg32 parameter);
x86::reg32 destroyWindow(x86::reg32 window);
std::int32_t defaultWindowProcedure(App* app, x86::reg32 window, x86::reg32 message, x86::reg32 wParam,
                                    x86::reg32 lParam);
x86::reg32 showWindow(x86::reg32 window, int how);
x86::reg32 setWindowPosition(x86::reg32 window, x86::reg32 after, int x, int y, int width, int height,
                             x86::reg32 flags);
x86::reg32 clientRect(x86::reg32 window, Rect* rect);
x86::reg32 clientToScreen(x86::reg32 window, Point* point);
x86::reg32 adjustWindowRect(Rect* rect, x86::reg32 style, x86::reg32 menu);
std::int32_t windowLong(x86::reg32 window, int index);
x86::reg32 isIconic(x86::reg32 window);
x86::reg32 setForegroundWindow(x86::reg32 window);
x86::reg32 setActiveWindow(x86::reg32 window);
x86::reg32 findWindow(x86::reg32 className, x86::reg32 windowName);
x86::reg32 desktopWindow();
x86::reg32 windowDC(x86::reg32 window);
x86::reg32 beginPaint(x86::reg32 window, x86::reg32 paint);
x86::reg32 endPaint(x86::reg32 window, x86::reg32 paint);
x86::reg32 loadIcon(x86::reg32 instance, x86::reg32 name);

/* The next message, waiting for one: the guest context let go while it waits. */
x86::reg32 getMessage(App* app, x86::CPU& cpu, Message* message, x86::reg32 window, x86::reg32 filterMin,
                      x86::reg32 filterMax);
x86::reg32 translateMessage(const Message* message);
/* The message handed to the window procedure, on the guest's stack as stdcall; its answer. */
std::int32_t dispatchMessage(App* app, x86::CPU& cpu, const Message* message);
x86::reg32 postMessage(x86::reg32 window, x86::reg32 message, x86::reg32 wParam, x86::reg32 lParam);
x86::reg32 sendNotifyMessage(x86::reg32 window, x86::reg32 message, x86::reg32 wParam, x86::reg32 lParam);
x86::reg32 postThreadMessage(x86::reg32 thread, x86::reg32 message, x86::reg32 wParam, x86::reg32 lParam);
/* The quit message posted; told to the log with the word the stack has on top (the caller's). */
void postQuitMessage(App* app, x86::CPU& cpu, int code);
x86::reg32 setWindowsHook(int kind, x86::reg32 procedure, x86::reg32 module, x86::reg32 thread);
x86::reg32 unhookWindowsHook(x86::reg32 hook);
std::int32_t callNextHook(x86::reg32 hook, int code, x86::reg32 wParam, x86::reg32 lParam);

// The cursor and the keys (user32).
int showCursor(x86::reg32 show);
x86::reg32 setCursor(x86::reg32 cursor);
x86::reg32 setCursorPosition(int x, int y);
x86::reg32 swapMouseButton(x86::reg32 swap);
/* A key's state, the high bit set while it is down (as a 16-bit value, sign extended into eax). */
std::int16_t asyncKeyState(int key);
std::int16_t keyState(int key);
int keyboardType(int which);
void keyboardEvent(std::uint8_t key, std::uint8_t scan, x86::reg32 flags, x86::reg32 extra);

// What the system says (user32, gdi32).
int systemMetrics(int index);
x86::reg32 systemParameters(x86::reg32 action, x86::reg32 parameter, x86::reg32* value, x86::reg32 profile);
int deviceCaps(x86::reg32 dc, int index);
x86::reg32 stockObject(int which);

}  // namespace host
}  // namespace nfs3hp

#endif
