#include <winapi/winmm.h>
#include <lib/timer.h>
#include <lib/gamepad.h>
#include <lib/window.h>
#include <x86.h>

namespace win32 { namespace winmm
{

UINT joyGetNumDevs(WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    return Gamepad::getCount();
}

MMRESULT timeBeginPeriod(WinApplication* app, x86::CPU& cpu,
                         UINT uPeriod)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(uPeriod);
    return 0;
}

MMRESULT timeEndPeriod(WinApplication* app, x86::CPU& cpu,
                       UINT uPeriod)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(uPeriod);
    return 0;
}

MMRESULT timeGetDevCaps(WinApplication* app, x86::CPU& cpu,
                        LPTIMECAPS ptc, UINT cbtc)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_ASSERT(cbtc == sizeof(*ptc));
    ptc->wPeriodMin = 1;
    ptc->wPeriodMax = 1000;
    return 0;
}

MMRESULT timeKillEvent(WinApplication* app, x86::CPU& cpu,
                       UINT uTimerID)
{
    NFS2_USE(cpu);
    win32::Timer* timer = dynamic_cast<win32::Timer*>(app->getResource(uTimerID));
    NFS_MSG_TRACE("timeKillEvent id=%u found=%d", unsigned(uTimerID), int(timer != nullptr));
    if (timer)
    {
        timer->cancel();
        app->freeResource(uTimerID);
    }
    return 0;
}

MMRESULT timeSetEvent(WinApplication* app, x86::CPU& cpu,
                      UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc,
                      x86::reg32 dwUser, UINT fuEvent)
{
    NFS2_USE(cpu);
    /* fuEvent bit 0 is TIME_PERIODIC; 0 is TIME_ONESHOT, which is all this
     * port implements (Timer::timerCallback returns 0, so SDL fires it once
     * and drops it).  That is only faithful if the game re-arms the timer
     * itself from inside the callback.  Since NFS2_ASSERT stopped being fatal,
     * a TIME_PERIODIC request would quietly degrade into a single shot -- which
     * would look exactly like "nothing updates unless I press a key", so the
     * arguments are logged before the assert, not instead of it. */
    NFS2_ASSERT(fuEvent == 0);
    NFS2_USE(uResolution);
    if (!cpu.terminate)
    {
        win32::Timer* timer = new win32::Timer(uDelay, app, lpTimeProc, dwUser);
        const x86::reg32 id = app->allocateResource(timer);
#ifdef NFS_TRACE_MSG
        /* Rate limited -- the game re-arms at ~126 Hz, so the hit counter plus
         * the timestamp is what gives the rate; a screen that stops arming
         * shows up as the counter going quiet.  A TIME_PERIODIC request is
         * never rate limited: it is rare, unsupported here, and would look
         * exactly like "nothing moves unless I press a key". */
        static unsigned hits = 0;
        if ((fuEvent & 1u) || win32::msgTraceRateLimit(hits))
            NFS_MSG_TRACE("timeSetEvent  id=%u delay=%ums res=%ums fuEvent=%u%s proc=0x%08x user=0x%08x (arm %u)",
                          unsigned(id), unsigned(uDelay), unsigned(uResolution), unsigned(fuEvent),
                          (fuEvent & 1u) ? " PERIODIC(UNSUPPORTED)" : " ONESHOT",
                          unsigned(lpTimeProc), unsigned(dwUser), hits);
#endif
        return id;
    }
    else
        return 0;
}

}}
