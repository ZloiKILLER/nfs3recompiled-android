#include <lib/timer.h>
#include <lib/window.h>
#include <atomic>
#include <time.h>
#ifdef _WIN32
# define localtime_r(a,b)  localtime_s(b,a)
#endif

namespace win32
{

void getSystemTime(SYSTEMTIME* systemTime)
{
    time_t now;
    struct tm localTime;
    time(&now);
    localtime_r(&now, &localTime);
    // tm_year counts from 1900, SYSTEMTIME.wYear wants the full year.
    systemTime->wYear = WORD(localTime.tm_year + 1900);
    systemTime->wMonth = WORD(localTime.tm_mon + 1);
    systemTime->wDayOfWeek = WORD(localTime.tm_wday);
    systemTime->wDay = WORD(localTime.tm_mday);
    systemTime->wHour = WORD(localTime.tm_hour);
    systemTime->wMinute = WORD(localTime.tm_min);
    systemTime->wSecond = WORD(localTime.tm_sec);
    systemTime->wMilliseconds = 0;
}

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

x86::reg32 timeGetTickCount()
{
    const Uint64 now = s_suspended.load() ? s_suspendedAt.load() : SDL_GetTicks();
    x86::reg32 result = (x86::reg32)(now - s_suspendedFor.load());
    return result;
}

Timer::Timer(x86::reg32 delay, WinApplication* app, x86::reg32 proc, x86::reg32 param)
    :   m_app(app)
    ,   m_callback(proc)
    ,   m_parameter(param)
    ,   m_id(SDL_AddTimer(delay, &timerCallback, this))
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
    x86::CPU cpu;
    //SDL_Log("Timer callback: %d", t->m_id);
#ifdef NFS_TRACE_MSG
    /* Whether this fires once or keeps coming back is the whole question for a
     * TIME_ONESHOT timer the game is expected to re-arm; the sdl_id and proc
     * identify which timer, so one shared rate limiter is enough.  Returning 0
     * below is what makes it one-shot. */
    static unsigned hits = 0;
    const bool trace = win32::msgTraceRateLimit(hits);
    if (trace)
        NFS_MSG_TRACE("timer fire    sdl_id=%u proc=0x%08x param=0x%08x (fire %u) -> guest",
                      unsigned(t->m_id), unsigned(t->m_callback), unsigned(t->m_parameter), hits);
#endif
    t->m_app->runCallback(cpu, t->m_callback, t->m_parameter);
#ifdef NFS_TRACE_MSG
    if (trace)
        NFS_MSG_TRACE("timer done    sdl_id=%u proc=0x%08x (fire %u), not re-armed by us",
                      unsigned(t->m_id), unsigned(t->m_callback), hits);
#endif
    //SDL_Log("Timer callback done: %d", t->m_id);
    return 0;
}

}
