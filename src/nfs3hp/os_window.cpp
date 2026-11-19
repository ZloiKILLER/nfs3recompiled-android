#include "os_window.h"

#include <lib/thread.h>
#include <lib/window.h>

#include <SDL3/SDL.h>

#include <cstdint>

/* The window's events come from SDL's one queue: the keys and the text SDL gives, and what the
 * port pushes there itself -- the game's messages (nfs::g_gameMessageEvent), the port's own word to
 * stop or to let a window go (nfs::g_portEvent), a character a key gave (nfs::g_characterEvent),
 * and a thread woken to stop (nfs::g_threadStopEvent, Thread::terminate).  The wait blocks in
 * SDL_WaitEvent: the original's loop blocked in GetMessageA too, so anything that repaints
 * without input has to post into the queue from somewhere else. */
namespace nfs3hp
{
namespace os
{
namespace window
{

namespace
{

// The port's own events under nfs::g_portEvent, by their code.
constexpr int kStop = 0, kClosed = 1;

/* The keys down, by virtual-key code, as the window's thread has taken them. */
bool s_keyDown[256];

/* The last key's scan code, for the characters typed after it, and whether a key came at all:
 * a character typed before any key counts 0 times. */
nfs::u32 s_lastScan = 0;
bool s_keyCame = false;

/* SDL's scan codes to set 1's, for the first 100. */
const nfs::u8 kScanCodes[100] =
{
    0x00, 0x00, 0x00, 0x00, 0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21,
    0x22, 0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18, 0x19,
    0x10, 0x13, 0x1F, 0x14, 0x16, 0x2F, 0x11, 0x2D, 0x15, 0x2C,
    0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x1C, 0x01, 0x0E, 0x0F, 0x39, 0x0C, 0x0D, 0x1A, 0x1B, 0x2B,
    0x00, 0x27, 0x28, 0x29, 0x33, 0x34, 0x35, 0x3A, 0x3B, 0x3C,
    0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x57, 0x58,
    0x37, 0x46, 0x59, 0x52, 0x47, 0x49, 0x53, 0x4F, 0x51, 0x4D,
    0x4B, 0x50, 0x48, 0x45, 0x35, 0x37, 0x4A, 0x4E, 0x1C, 0x4F,
    0x50, 0x51, 0x4B, 0x4C, 0x4D, 0x47, 0x48, 0x49, 0x52, 0x53
};

#ifdef NFS_TRACE_MSG
/* Woken for nothing, per waiting thread: SDL_WaitEvent returning is not the same as the game
 * being told something.  Thread-targeted wakeups that belonged to another thread are counted
 * apart: the port has one SDL queue, so every waiting thread wakes for all of them. */
thread_local unsigned s_ignoredEvents = 0;
thread_local unsigned s_foreignWakes = 0;
#endif

void push(Uint32 type, int code, nfs::u32 first, nfs::u32 second, Handle window)
{
    SDL_Event event;
    SDL_zero(event);
    event.type = type;
    event.user.code = code;
    event.user.data1 = reinterpret_cast<void*>(static_cast<intptr_t>(first));
    event.user.data2 = reinterpret_cast<void*>(static_cast<intptr_t>(second));
    event.user.windowID = window;
    SDL_PushEvent(&event);
}

nfs::u32 word(void* data)
{
    return static_cast<nfs::u32>(reinterpret_cast<intptr_t>(data));
}

/* An SDL key's virtual-key code: the keys that have their own, else the key's code itself when it
 * is a character's (its letters still small).  0 for a key that has neither. */
nfs::u32 virtualKeyOf(SDL_Keycode key)
{
    switch (key)
    {
    case SDLK_LSHIFT:
    case SDLK_RSHIFT: return 0x10;
    case SDLK_PAGEDOWN: return 0x21;
    case SDLK_PAGEUP: return 0x22;
    case SDLK_END: return 0x23;
    case SDLK_HOME: return 0x24;
    case SDLK_LEFT: return 0x25;
    case SDLK_UP: return 0x26;
    case SDLK_RIGHT: return 0x27;
    case SDLK_DOWN: return 0x28;
    case SDLK_INSERT: return 0x2D;
    case SDLK_DELETE: return 0x2E;
    case SDLK_KP_0: return 0x60;
    case SDLK_KP_1: return 0x61;
    case SDLK_KP_2: return 0x62;
    case SDLK_KP_3: return 0x63;
    case SDLK_KP_4: return 0x64;
    case SDLK_KP_5: return 0x65;
    case SDLK_KP_6: return 0x66;
    case SDLK_KP_7: return 0x67;
    case SDLK_KP_8: return 0x68;
    case SDLK_KP_9: return 0x69;
    case SDLK_KP_MULTIPLY: return 0x6A;
    case SDLK_KP_PLUS: return 0x6B;
    case SDLK_KP_MINUS: return 0x6D;
    case SDLK_KP_PERIOD: return 0x6E;
    case SDLK_KP_DIVIDE: return 0x6F;
    case SDLK_EQUALS: return 0xBB;
    case SDLK_MINUS: return 0xBD;
    case SDLK_PERIOD: return 0xBE;
    case SDLK_GRAVE: return 0xC0;
    case SDLK_APOSTROPHE: return 0xDE;
    case SDLK_KP_ENTER: return SDLK_RETURN;
    default: return (key & 0x40000000) == 0 ? nfs::u32(key) : 0;
    }
}

/* A key pressed or let go, as the thread takes it: its virtual-key code (letters as capitals), its
 * scan code and the key table.  A control character's key (backspace to escape) pressed also
 * types it: the character is queued behind the key. */
Event keyEvent(const SDL_KeyboardEvent& key, bool down)
{
    const nfs::u32 virtualKey = virtualKeyOf(key.key);
    Event event;
    event.kind = Event::Kind::key;
    event.down = down;
    event.repeats = 1;
    event.scan = key.scancode < 100 ? kScanCodes[key.scancode] : 0;
    s_lastScan = event.scan;
    s_keyCame = true;
    const SDL_Keycode typed = key.key == SDLK_KP_ENTER ? SDLK_RETURN : key.key;
    if (down && virtualKey == nfs::u32(typed) && typed >= SDLK_BACKSPACE && typed < SDLK_SPACE)
        push(nfs::g_characterEvent, 0, virtualKey, event.scan, 0);
    event.key = key.key >= SDLK_A && key.key <= SDLK_Z ? (key.key & ~0x20u) : virtualKey;
    if (event.key < 256)
        s_keyDown[event.key] = down;
    return event;
}

Event waitImpl(nfs::ThreadState& ts)
{
    SDL_Event event;
    for (;;)
    {
        if (!SDL_WaitEvent(&event))
            continue;
        if (ts.terminate)
            return Event();
        if (event.type == nfs::g_gameMessageEvent)
        {
            Event message;
            message.kind = Event::Kind::message;
            message.window = event.user.windowID;
            message.message = nfs::u32(event.user.code);
            message.first = word(event.user.data1);
            message.second = word(event.user.data2);
            return message;
        }
        if (event.type == nfs::g_portEvent)
        {
            Event own;
            if (event.user.code == kClosed)
            {
                own.kind = Event::Kind::closed;
                own.window = event.user.windowID;
            }
            return own;
        }
        if (event.type == nfs::g_threadStopEvent)
        {
            if (word(event.user.data1) == nfs::Thread::currentThreadId())
                return Event();
#ifdef NFS_TRACE_MSG
            ++s_foreignWakes;
#endif
            continue;
        }
        if (event.type == nfs::g_characterEvent)
        {
            Event typed;
            typed.kind = Event::Kind::character;
            typed.character = word(event.user.data1);
            typed.scan = word(event.user.data2);
            typed.repeats = 1;
            return typed;
        }
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            // Said, because a game that closes by itself leaves nothing else behind.
            SDL_Log("[EXIT] SDL asked the game to quit");
            return Event();
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            return keyEvent(event.key, event.type == SDL_EVENT_KEY_DOWN);
        /* No gamepad case here, and none is wanted: sdl-backend/gamepad.cpp polls the pads,
         * pushes their buttons as the key events above and hands their sticks and triggers to
         * the game's controls as a joystick's axes.  Joystick and gamepad events could not be
         * used anyway -- a Gamepad, made when the game's controls open the pads, calls
         * SDL_SetJoystickEventsEnabled(false), which starves SDL3's joystick-to-gamepad event
         * watcher before a single frame is drawn. */
        case SDL_EVENT_TEXT_INPUT:
        {
            // The phone's keyboard: the text's first byte, with the last key's scan code.
            Event typed;
            typed.kind = Event::Kind::character;
            typed.character = nfs::u32(event.text.text[0]);
            typed.scan = s_lastScan;
            typed.repeats = s_keyCame ? 1 : 0;
            return typed;
        }
        default:
#ifdef NFS_TRACE_MSG
            ++s_ignoredEvents;
#endif
            break;
        }
    }
}

}  // namespace

Area clientArea(Handle window)
{
    NFS2_USE(window);
    return { 0, 0, kScreenWidth, kScreenHeight };
}

Handle open(App* app, const char* title, std::int32_t width, std::int32_t height)
{
    return app->allocateResource(new nfs::Window(title, 0, 0, width, height));
}

void close(Handle window)
{
    NFS_MSG_TRACE("window close  -> window=%u", unsigned(window));
    push(nfs::g_portEvent, kClosed, 0, 0, window);
}

Event wait(App* app, nfs::ThreadState& ts)
{
#ifdef NFS_TRACE_MSG
    const Uint64 waitStart = SDL_GetTicks();
    const unsigned framesAtEntry = nfs::g_traceFrames;
    s_ignoredEvents = 0;
    s_foreignWakes = 0;
#endif
    app->unlockContext(ts);
    const Event event = waitImpl(ts);
    app->lockContext(ts);
#ifdef NFS_TRACE_MSG
    /* blocked= is the point: a wait that returns at once every time is being fed by something,
     * one that sits at 3000 ms is waiting for input nobody sends.  frames_meanwhile= says
     * whether the picture moved while it waited. */
    nfs::eventTrace("wait <- kind=%d key=0x%02x char=0x%02x msg=0x%04x first=0x%08x second=0x%08x window=%u "
                    "blocked=%ums frames_meanwhile=%u ignored=%u foreign_wakes=%u",
                    int(event.kind), unsigned(event.key), unsigned(event.character), unsigned(event.message),
                    unsigned(event.first), unsigned(event.second), unsigned(event.window),
                    unsigned(SDL_GetTicks() - waitStart), nfs::g_traceFrames - framesAtEntry, s_ignoredEvents,
                    s_foreignWakes);
#endif
    return event;
}

void post(Handle window, nfs::u32 message, nfs::u32 first, nfs::u32 second)
{
    NFS_MSG_TRACE("post          -> msg=0x%04x first=0x%08x second=0x%08x window=%u", unsigned(message),
                  unsigned(first), unsigned(second), unsigned(window));
    push(nfs::g_gameMessageEvent, int(message), first, second, window);
}

void quit(nfs::u32 from)
{
    if (from != 0)
        SDL_Log("[EXIT] the window's thread told to stop from 0x%x", unsigned(from));
    push(nfs::g_portEvent, kStop, 0, 0, 0);
}

bool keyDown(nfs::u32 virtualKey)
{
#ifdef NFS_TRACE_MSG
    static unsigned hits = 0;
    if (nfs::eventTraceRateLimit(hits))
        NFS_MSG_TRACE("keyDown vKey=0x%02x down=%d (hit %u)", unsigned(virtualKey),
                      int(virtualKey < 256 && s_keyDown[virtualKey]), hits);
#endif
    return virtualKey < 256 && s_keyDown[virtualKey];
}

}  // namespace window
}  // namespace os
}  // namespace nfs3hp
