#include "host_threads.h"

#include <lib/event.h>
#include <lib/mutex.h>
#include <lib/thread.h>
#include <lib/timer.h>
#include <lib/window.h>

#include <SDL3/SDL.h>
#include <vector>
#ifndef _WIN32
# include <unwind.h>
#endif

/* What the port's Windows (kernel32's and winmm's functions of the same names) did, moved as it was. */
namespace nfs3hp
{
namespace host
{

void initLock(App* app, win32::Mutex** section)
{
    *section = new win32::Mutex("");
    app->allocateResource(*section);
}

void deleteLock(App* app, win32::Mutex** section)
{
    app->freeResource((*section)->getResourceIndex());
}

void enterLock(App* app, x86::CPU& cpu, win32::Mutex** section)
{
    /* An uncontended critical section -- overwhelmingly the common case, since
     * most Win32 critical sections guard something brief -- used to always pay
     * for releasing and reacquiring the single global guest execution context
     * anyway.  That context is shared by every guest thread, so on a screen
     * where more than one thread hammers a critical section at high frequency
     * (observed on device: one thread alone doing 18000+ Enter/GetTickCount/
     * Leave triples per second while the Camera/Graphics settings screens are
     * open, other threads doing the same at the same time), every single call
     * became a real OS mutex release-then-reacquire against contention from
     * every other such thread -- a thundering-herd on the context mutex, not
     * on the critical section it was nominally protecting.  Try the fast,
     * non-blocking path first; only fall back to the release-context dance
     * when the critical section is genuinely held by someone else, which is
     * exactly the case that needs it (another guest thread may itself be
     * blocked waiting for the execution context, so it cannot be allowed to
     * make progress and release the critical section unless we let go). */
    if ((*section)->tryLock())
        return;
    app->unlockContext(cpu);
    (*section)->lock();
    app->lockContext(cpu);
}

void leaveLock(App* app, x86::CPU& cpu, win32::Mutex** section)
{
    (*section)->unlock();
    app->unlockContext(cpu);
    app->lockContext(cpu);
}

x86::reg32 createThread(App* app, x86::reg32 attributes, x86::reg32 stackBytes, x86::reg32 entry,
                        x86::reg32 parameter, x86::reg32 flags, x86::reg32* threadId)
{
    NFS2_USE(attributes);
    NFS2_ASSERT(stackBytes <= 1024*1024);
    win32::Thread* thread = new win32::Thread(app, entry, parameter, flags);
    if ((flags & 0x4) == 0) // CREATE_SUSPENDED
        thread->resume();
    if (threadId)
        *threadId = thread->threadId();
    const x86::reg32 h = app->allocateResource(thread);
    if (App::traceApi())
    {
        SDL_Log("[API] CreateThread entry=0x%x param=0x%x flags=0x%x -> h=%u tid=%u",
                unsigned(entry), unsigned(parameter), unsigned(flags),
                unsigned(h), unsigned(thread->threadId()));
    }
    return h;
}

void exitThread(App* app, x86::CPU& cpu, x86::reg32 code)
{
    NFS2_USE(app);
    NFS2_USE(code);
    cpu.terminate = true;
}

x86::reg32 resumeThread(App* app, x86::reg32 thread)
{
    dynamic_cast<win32::Thread*>(app->getResource(thread))->resume();
    return 1;
}

x86::reg32 terminateThread(App* app, x86::CPU& cpu, x86::reg32 thread, x86::reg32 code)
{
    NFS2_USE(code);
    win32::Thread* const t = dynamic_cast<win32::Thread*>(app->getResource(thread));
    app->unlockContext(cpu);
    t->terminate();
    app->lockContext(cpu);
    return 0;
}

int threadPriority(x86::reg32 thread)
{
    NFS2_USE(thread);
    NFS2_ASSERT(false);
    return 0;
}

x86::reg32 setThreadPriority(x86::reg32 thread, int priority)
{
    NFS2_USE(thread);
    NFS2_USE(priority);
    return 1;
}

x86::reg32 currentThread(App* app)
{
    return app->allocateResource(new win32::Thread());
}

x86::reg32 currentThreadId(App* app, x86::CPU& cpu)
{
    return app->getMemory<x86::reg32>(cpu.efs + 0x4);
}

x86::reg32 duplicateThread(App* app, x86::reg32 sourceProcess, x86::reg32 source, x86::reg32 targetProcess,
                           x86::reg32* target, x86::reg32 access, x86::reg32 inherit, x86::reg32 options)
{
    NFS2_USE(sourceProcess);
    NFS2_USE(targetProcess);
    NFS2_USE(access);
    NFS2_USE(inherit);
    NFS2_USE(options);
    win32::GenericResource* const resource = dynamic_cast<win32::GenericResource*>(app->getResource(source));
    NFS2_ASSERT(dynamic_cast<win32::Thread*>(resource));
    *target = app->allocateResource(dynamic_cast<win32::Thread*>(resource)->duplicate());
    return 1;
}

x86::reg32 tlsAlloc()
{
    NFS2_ASSERT(false);
    return 0;
}

x86::reg32 tlsFree(x86::reg32 index)
{
    NFS2_USE(index);
    NFS2_ASSERT(false);
    return 0;
}

x86::reg32 tlsGet(x86::reg32 index)
{
    NFS2_USE(index);
    NFS2_ASSERT(false);
    return 0;
}

x86::reg32 tlsSet(x86::reg32 index, x86::reg32 value)
{
    NFS2_USE(index);
    NFS2_USE(value);
    NFS2_ASSERT(false);
    return 0;
}

x86::reg32 createEvent(App* app, x86::reg32 attributes, x86::reg32 manualReset, x86::reg32 initialState,
                       const char* name)
{
    NFS2_USE(attributes);
    return app->allocateResource(new win32::Event(name, manualReset != 0, initialState != 0));
}

x86::reg32 setEvent(App* app, x86::reg32 event)
{
    win32::Event* const e = dynamic_cast<win32::Event*>(app->getResource(event));
    if (!e)
    {
        NFS2_ASSERT(false);
        return 0;
    }
    e->set();
    return 1;
}

x86::reg32 resetEvent(x86::reg32 event)
{
    NFS2_USE(event);
    NFS2_ASSERT(false);
    return 0;
}

x86::reg32 pulseEvent(App* app, x86::reg32 event)
{
    return dynamic_cast<win32::Event*>(app->getResource(event))->pulse();
}

x86::reg32 createMutex(App* app, x86::CPU& cpu, x86::reg32 attributes, x86::reg32 initialOwner, const char* name)
{
    NFS2_USE(attributes);
    win32::Mutex* const mutex = new win32::Mutex(name);
    if (initialOwner)
    {
        app->unlockContext(cpu);
        mutex->lock();
        app->lockContext(cpu);
    }
    return app->allocateResource(mutex);
}

x86::reg32 waitOne(App* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32 ms)
{
    app->unlockContext(cpu);
    win32::Event* event = dynamic_cast<win32::Event*>(app->getResource(handle));
    const x86::reg32 result = win32::Event::wait(cpu, &event, &event + 1, false, x86::sreg32(ms));
    app->lockContext(cpu);
    return result;
}

x86::reg32 waitMany(App* app, x86::CPU& cpu, x86::reg32 count, const x86::reg32* handles, x86::reg32 waitAll,
                    x86::reg32 ms, x86::reg32 alertable)
{
    NFS2_USE(alertable);
    std::vector<win32::Event*> events(count);
    for (x86::reg32 i = 0; i < count; ++i)
    {
        events[i] = dynamic_cast<win32::Event*>(app->getResource(handles[i]));
        if (!events[i])
        {
            NFS2_ASSERT(false);
            return 0xffffffff;
        }
    }
    app->unlockContext(cpu);
    const x86::reg32 result = win32::Event::wait(cpu, &events.front(), &events.front() + count, waitAll != 0,
                                                 x86::sreg32(ms));
    app->lockContext(cpu);
    return result;
}

x86::reg32 sleep(App* app, x86::CPU& cpu, x86::reg32 ms, x86::reg32 alertable)
{
    NFS2_USE(alertable);
    app->unlockContext(cpu);
    win32::Thread::sleep(ms);
    app->lockContext(cpu);
    return 0;
}

x86::reg32 ticks(App* app, x86::CPU& cpu)
{
    /* Diagnostic, kept live: this rate-limit trips whenever some guest thread
     * calls GetTickCount 500+ times in 200ms, which is what a spin-wait poll
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
                    "[STREAMTHROTTLE] engaged: %d GetTickCount calls in 200ms", s_windowCalls);
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
        app->unlockContext(cpu);
        win32::Thread::sleep(1);
        app->lockContext(cpu);
    }
    return win32::timeGetTickCount();
}

void localTime(win32::SYSTEMTIME* time)
{
    win32::getSystemTime(time);
}

x86::reg32 timeZone(x86::reg32 information)
{
    NFS2_USE(information);
    NFS2_ASSERT(false);
    return 0;
}

x86::reg32 timerPeriodBegin(x86::reg32 period)
{
    NFS2_USE(period);
    return 0;
}

x86::reg32 timerPeriodEnd(x86::reg32 period)
{
    NFS2_USE(period);
    return 0;
}

x86::reg32 timerCaps(TimerCaps* caps, x86::reg32 bytes)
{
    NFS2_ASSERT(bytes == sizeof(*caps));
    caps->periodMin = 1;
    caps->periodMax = 1000;
    return 0;
}

x86::reg32 startTimer(App* app, x86::CPU& cpu, x86::reg32 delay, x86::reg32 resolution, x86::reg32 proc,
                      x86::reg32 user, x86::reg32 flags)
{
    /* flags bit 0 is TIME_PERIODIC; 0 is TIME_ONESHOT, which is all this
     * port implements (Timer::timerCallback returns 0, so SDL fires it once
     * and drops it).  That is only faithful if the game re-arms the timer
     * itself from inside the callback.  Since NFS2_ASSERT stopped being fatal,
     * a TIME_PERIODIC request would quietly degrade into a single shot -- which
     * would look exactly like "nothing updates unless I press a key", so the
     * arguments are logged before the assert, not instead of it. */
    NFS2_ASSERT(flags == 0);
    NFS2_USE(resolution);
    if (cpu.terminate)
        return 0;
    const x86::reg32 id = app->allocateResource(new win32::Timer(delay, app, proc, user));
#ifdef NFS_TRACE_MSG
    /* Rate limited -- the game re-arms at ~126 Hz, so the hit counter plus
     * the timestamp is what gives the rate; a screen that stops arming
     * shows up as the counter going quiet.  A TIME_PERIODIC request is
     * never rate limited: it is rare, unsupported here, and would look
     * exactly like "nothing moves unless I press a key". */
    static unsigned hits = 0;
    if ((flags & 1u) || win32::msgTraceRateLimit(hits))
        NFS_MSG_TRACE("timeSetEvent  id=%u delay=%ums res=%ums fuEvent=%u%s proc=0x%08x user=0x%08x (arm %u)",
                      unsigned(id), unsigned(delay), unsigned(resolution), unsigned(flags),
                      (flags & 1u) ? " PERIODIC(UNSUPPORTED)" : " ONESHOT",
                      unsigned(proc), unsigned(user), hits);
#endif
    return id;
}

x86::reg32 killTimer(App* app, x86::reg32 id)
{
    win32::Timer* const timer = dynamic_cast<win32::Timer*>(app->getResource(id));
    NFS_MSG_TRACE("timeKillEvent id=%u found=%d", unsigned(id), int(timer != nullptr));
    if (timer)
    {
        timer->cancel();
        app->freeResource(id);
    }
    return 0;
}

}  // namespace host
}  // namespace nfs3hp
