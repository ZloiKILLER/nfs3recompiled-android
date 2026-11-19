#include <lib/timer.h>
#include <lib/window.h>
#include <atomic>

namespace nfs
{

/* suspendTime(): whether the game's time stands still, since when, and how long
 * it has stood still before, all told -- which the clock leaves out. */
static std::atomic<bool> s_suspended{false};
static std::atomic<Uint64> s_suspendedAt{0};
static std::atomic<Uint64> s_suspendedFor{0};

void suspendTime(bool suspended)
{
    if (suspended == s_suspended.load())
        return;
    if (suspended)
    {
        s_suspendedAt.store(SDL_GetTicks());
        s_suspended.store(true);
    }
    else
    {
        const Uint64 stood = SDL_GetTicks() - s_suspendedAt.load();
        s_suspendedFor.fetch_add(stood);
        s_suspended.store(false);
        SDL_Log("[TIME] the game's time stood still for %.1f s in the background", double(stood) / 1000.0);
    }
}

/* NFS_TIME_SCALE (debug mode, Traces: replay_fast): the game's time runs this many
 * times as fast, for a replay to be checked sooner.  A replay's steps come from the
 * recording, not from the clock, so what it comes to is the same. */
static Uint32 timeScale()
{
    static const Uint32 scale = [] {
        const char* value = SDL_getenv("NFS_TIME_SCALE");
        const int asked = value ? SDL_atoi(value) : 1;
        return Uint32(asked > 1 ? asked : 1);
    }();
    return scale;
}

nfs::u32 gameClockMs()
{
    const Uint64 now = s_suspended.load() ? s_suspendedAt.load() : SDL_GetTicks();
    nfs::u32 result = (nfs::u32)((now - s_suspendedFor.load()) * timeScale());
    return result;
}

Timer::Timer(nfs::u32 delay, App* app, nfs::u32 proc, nfs::u32 param)
    :   m_app(app)
    ,   m_callback(proc)
    ,   m_parameter(param)
    ,   m_id(SDL_AddTimer(delay / timeScale() > 0 ? delay / timeScale() : 1, &timerCallback, this))
{
    //SDL_Log("Started timer %d (%d ms)", m_id, delay);
}

Timer::~Timer()
{
    cancel();
}

void Timer::cancel()
{
    NFS_MSG_TRACE("timer cancel  sdl_id=%u proc=0x%08x", unsigned(m_id), unsigned(m_callback));
    SDL_RemoveTimer(m_id);
}

Uint32 Timer::timerCallback(void* data, SDL_TimerID /*timerID*/, Uint32 interval)
{
    NFS2_USE(interval);
    Timer* t = (Timer*)data;
    /* In the background (suspendTime) the timer waits: asked again in 10 ms,
     * it fires once the app is back.  The game's timers are one-shot and armed
     * again by what they call, so one that never fired would stop them for
     * good; one that fires late only runs late. */
    if (s_suspended.load())
        return 10;
    nfs::ThreadState ts;
    //SDL_Log("Timer callback: %d", t->m_id);
#ifdef NFS_TRACE_MSG
    /* Whether this fires once or keeps coming back is the whole question for a
     * TIME_ONESHOT timer the game is expected to re-arm; the sdl_id and proc
     * identify which timer, so one shared rate limiter is enough.  Returning 0
     * below is what makes it one-shot. */
    static unsigned hits = 0;
    const bool trace = nfs::eventTraceRateLimit(hits);
    if (trace)
        NFS_MSG_TRACE("timer fire    sdl_id=%u proc=0x%08x param=0x%08x (fire %u) -> guest",
                      unsigned(t->m_id), unsigned(t->m_callback), unsigned(t->m_parameter), hits);
#endif
    t->m_app->runCallback(ts, t->m_callback, t->m_parameter);
#ifdef NFS_TRACE_MSG
    if (trace)
        NFS_MSG_TRACE("timer done    sdl_id=%u proc=0x%08x (fire %u), not re-armed by us",
                      unsigned(t->m_id), unsigned(t->m_callback), hits);
#endif
    //SDL_Log("Timer callback done: %d", t->m_id);
    return 0;
}

}
