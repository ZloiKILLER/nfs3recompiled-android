#include "os_time.h"

#include <lib/thread.h>
#include <lib/timer.h>
#include <lib/window.h>

#include <SDL3/SDL.h>
#include <time.h>
#ifdef _WIN32
# define localtime_r(a, b) localtime_s(b, a)
#else
# include <unwind.h>
#endif

namespace nfs3hp
{
namespace os
{
namespace time
{

nfs::u32 ticks(App* app, nfs::ThreadState& ts)
{
    /* Diagnostic, kept live: this rate-limit trips whenever some guest thread
     * asks for the time 500+ times in 200ms, which is what a spin-wait poll
     * loop looks like from here.  Two different capture sessions have now
     * shown this pattern -- once with real disk I/O happening concurrently on
     * a *different* thread (paused-race traffic-car streaming, unrelated
     * background noise it turned out), and once, from a clean main-menu-only
     * repro of the Camera/Graphics/HUD settings hang, with NO other I/O in
     * flight at all -- so whatever this second case is polling is not file
     * read completion, at least not directly.  The backtrace below exists to
     * find out what it actually is on the NEXT repro, since guessing from the
     * first (misleading) capture already cost real time once.  Applying a
     * real sleep while throttling is a no-op that helps if this specific
     * caller is a bounded livelock and does nothing otherwise; it was tested
     * against the live hang and did NOT resolve it, ruling out "just needs
     * a bigger OS scheduling slice" as the fix, whatever the root cause is. */
    static Uint64 s_windowStartMs = 0;
    static int s_windowCalls = 0;
    static bool s_throttling = false;
    static bool s_dumped = false;
    const Uint64 now = SDL_GetTicks();
    if (now - s_windowStartMs > 200)
    {
        if (s_throttling && s_windowCalls < 200)
        {
            s_throttling = false;
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[STREAMTHROTTLE] released");
        }
        s_windowStartMs = now;
        s_windowCalls = 0;
    }
    if (++s_windowCalls == 500 && !s_throttling)
    {
        s_throttling = true;
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                    "[STREAMTHROTTLE] engaged: %d calls for the time in 200ms", s_windowCalls);
        if (!s_dumped)
        {
            s_dumped = true;
#ifndef _WIN32
            struct Ctx { void* frames[16]; int n; };
            Ctx ctx = { {}, 0 };
            _Unwind_Backtrace(
                [](_Unwind_Context* uc, void* arg) -> _Unwind_Reason_Code
                {
                    Ctx* c = static_cast<Ctx*>(arg);
                    if (c->n < 16)
                    {
                        c->frames[c->n++] = reinterpret_cast<void*>(_Unwind_GetIP(uc));
                    }
                    return c->n < 16 ? _URC_NO_REASON : _URC_END_OF_STACK;
                },
                &ctx);
            SDL_Log("[HANGDIAG] %d backtrace frames:", ctx.n);
            for (int i = 0; i < ctx.n; ++i)
            {
                SDL_Log("[HANGDIAG]   frame %d: %p", i, ctx.frames[i]);
            }
#endif
        }
    }
    if (s_throttling)
    {
        app->unlockContext(ts);
        nfs::Thread::sleep(1);
        app->lockContext(ts);
    }
    return nfs::gameClockMs();
}

Date now()
{
    time_t seconds;
    struct tm local;
    ::time(&seconds);
    localtime_r(&seconds, &local);
    Date date;
    date.year = nfs::u32(local.tm_year + 1900);  // tm_year counts from 1900
    date.month = nfs::u32(local.tm_mon + 1);
    date.dayOfWeek = nfs::u32(local.tm_wday);
    date.day = nfs::u32(local.tm_mday);
    date.hour = nfs::u32(local.tm_hour);
    date.minute = nfs::u32(local.tm_min);
    date.second = nfs::u32(local.tm_sec);
    date.millisecond = 0;
    return date;
}

}  // namespace time

namespace timers
{

std::optional<Handle> start(App* app, nfs::ThreadState& ts, nfs::u32 delay, nfs::u32 callback, nfs::u32 user)
{
    /* One shot: Timer::timerCallback returns 0, so SDL fires it once and drops it.  The game sets
     * the next one from inside the callback. */
    if (ts.terminate)
        return std::nullopt;
    const Handle handle = app->allocateResource(new nfs::Timer(delay, app, callback, user));
#ifdef NFS_TRACE_MSG
    /* Rate limited -- the game sets one at ~126 Hz, so the hit counter plus the timestamp is what
     * gives the rate; a screen that stops setting them shows up as the counter going quiet. */
    static unsigned hits = 0;
    if (nfs::eventTraceRateLimit(hits))
        NFS_MSG_TRACE("timer start  id=%u delay=%ums proc=0x%08x user=0x%08x (arm %u)", unsigned(handle),
                      unsigned(delay), unsigned(callback), unsigned(user), hits);
#endif
    if (ts.terminate)
        return std::nullopt;
    return handle;
}

void stop(App* app, Handle timer)
{
    nfs::Timer* const t = dynamic_cast<nfs::Timer*>(app->getResource(timer));
    NFS_MSG_TRACE("timer stop   id=%u found=%d", unsigned(timer), int(t != nullptr));
    if (t)
    {
        t->cancel();
        app->freeResource(timer);
    }
}

}  // namespace timers

}  // namespace os
}  // namespace nfs3hp
