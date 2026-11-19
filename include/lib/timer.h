#ifndef LIB_TIMER_H_
#define LIB_TIMER_H_

#include <lib/app.h>
#include <SDL3/SDL.h>

namespace nfs
{

nfs::u32 gameClockMs();

/* The game's time stops while the app is in the background: the clock it reads
 * (gameClockMs) stands still, and its timers wait to fire until the app is
 * back.  Otherwise the timer went on counting the game's ticks while the game
 * itself was stopped, and the game made up for all of them on its return --
 * a race went on without the player.  Called with true on the way to the
 * background, false on the way back. */
void suspendTime(bool suspended);

class Timer : public GenericResource
{
public:
    Timer(nfs::u32 delay, App* app, nfs::u32 proc, nfs::u32 param);
    ~Timer();

    void cancel();
private:
    static Uint32 timerCallback(void* data, SDL_TimerID timerID, Uint32 interval);

private:
    App* m_app;
    nfs::u32      m_callback;
    nfs::u32      m_parameter;
    SDL_TimerID     m_id;
};

}

#endif

