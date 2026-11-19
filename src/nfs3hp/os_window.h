#ifndef NFS3HP_OS_WINDOW_H
#define NFS3HP_OS_WINDOW_H

#include "os_system.h"

#include <game_types.h>
#include <lib/app.h>

#include <cstdint>

/* The port's window, as the game's code needs it: one window, the whole screen, made for the
 * game's window thread, and what reaches that thread -- the keys, what is typed, the game's own
 * messages to itself, the window let go, the word to stop -- as the port's events, which the
 * game's thread hands to its own message table (native_frontend_wina.cpp).  Plain C++ on SDL and
 * src/lib's Window. */
namespace nfs3hp
{
namespace os
{
namespace window
{

/* The screen the game is told it has, whatever the phone's: the window covers it. */
constexpr std::int32_t kScreenWidth = 800;
constexpr std::int32_t kScreenHeight = 600;

/* Whether the machine has a network: it has (without one the game offers no TCP/IP). */
constexpr bool kNetworkPresent = true;

/* A rectangle on the screen: its left and top, and past its right and bottom. */
struct Area
{
    std::int32_t left;
    std::int32_t top;
    std::int32_t right;
    std::int32_t bottom;
};

/* The window `window`'s client area on the screen: the whole screen, for every window. */
Area clientArea(Handle window);

/* A window, titled `title`, `width` by `height`: its handle. */
Handle open(App* app, const char* title, std::int32_t width, std::int32_t height);

/* The window let go, once its thread has taken the events before (Event::Kind::closed). */
void close(Handle window);

/* What the window's thread is told, the next in turn. */
struct Event
{
    enum class Kind
    {
        stop,       // the thread stops waiting: the game is to end, or the thread itself is
        key,        // a key pressed or let go
        character,  // a character typed
        closed,     // the window `window` let go (close)
        message,    // a message of the game's own (post)
    };

    Kind kind = Kind::stop;
    bool down = false;          // key: pressed, not let go
    nfs::u32 key = 0;           // key: its virtual-key code, the code the game's tables know keys by
    nfs::u32 character = 0;     // character: what was typed
    nfs::u32 scan = 0;          // key, character: the key's scan code (set 1), 0 for one without
    nfs::u32 repeats = 0;       // key, character: how many times it counts -- 1, or for a character
                                // typed before any key came, 0
    Handle window = 0;          // closed, message: the window it is for; 0 for a key or a character
    nfs::u32 message = 0;       // message: the game's own number for it, and its two words
    nfs::u32 first = 0;
    nfs::u32 second = 0;
};

/* The next event, waited for: the guest context let go while it waits.  An event the thread
 * is not the one for (another thread woken to stop) is passed over. */
Event wait(App* app, nfs::ThreadState& ts);

/* The game's message `message`, with its two words, to the thread of the window `window`. */
void post(Handle window, nfs::u32 message, nfs::u32 first, nfs::u32 second);

/* The window's thread told to stop (Event::Kind::stop).  `from`, when not 0, is the game's code
 * that asked, said in the log: a game that closes by itself leaves nothing else behind to tell
 * why. */
void quit(nfs::u32 from = 0);

/* Whether the key `virtualKey` is down, as the window's thread has taken the keys so far. */
bool keyDown(nfs::u32 virtualKey);

}  // namespace window
}  // namespace os
}  // namespace nfs3hp

#endif
