#include <lib/glthread.h>
#include <lib/renderer.h>
#include <SDL3/SDL.h>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

namespace win32
{
namespace glthread
{

namespace
{

struct Job
{
    Renderer*             renderer;
    std::function<void()> work;
};

std::mutex              s_mutex;
std::condition_variable s_wake;     // work posted, or a release asked for
std::condition_variable s_idle;     // the queue ran dry, a release done, a present done
std::deque<Job>         s_jobs;
bool                    s_busy = false;
bool                    s_releasing = false;
int                     s_presents = 0;
std::thread::id         s_threadId;
/* The renderer whose context the GL thread has current; its own. */
Renderer*               s_current = nullptr;

void loop()
{
    std::unique_lock<std::mutex> lock(s_mutex);
    for (;;)
    {
        s_wake.wait(lock, []() { return !s_jobs.empty() || s_releasing; });
        if (s_jobs.empty())
        {
            if (s_current)
            {
                s_current->clearCurrent();
                s_current = nullptr;
            }
            s_releasing = false;
            s_idle.notify_all();
            continue;
        }
        Job job = std::move(s_jobs.front());
        s_jobs.pop_front();
        s_busy = true;
        lock.unlock();
        if (s_current != job.renderer)
        {
            job.renderer->setCurrent();
            s_current = job.renderer;
        }
        job.work();
        job.work = nullptr;
        lock.lock();
        s_busy = false;
        if (s_jobs.empty())
            s_idle.notify_all();
    }
}

/* Called on the thread that pumps the events.
 *
 * Going to the background: SDL has the window's surface freed, which it can
 * only do while no thread has a context current on it.  But Android destroys
 * the surface before the game has pumped the pause, and SDL lets the game draw
 * one frame more after it: the GL thread made the context current once more,
 * on the dead surface, and held it through the pause.  Back in the foreground
 * SDL made a new surface, and the GL thread, its context current all along,
 * went on drawing into the old one -- nothing on the screen (2026-09-29).  So
 * it lets go once more when the app is back, SDL's new surface ready, and
 * takes the context up again with the next work, on that surface. */
bool SDLCALL lifecycleWatch(void* /*userdata*/, SDL_Event* event)
{
    if (event->type == SDL_EVENT_WILL_ENTER_BACKGROUND || event->type == SDL_EVENT_DID_ENTER_FOREGROUND)
    {
        release();
        SDL_Log("[GFX] %s: the GL thread let go of its context",
                event->type == SDL_EVENT_WILL_ENTER_BACKGROUND ? "to the background" : "back in the foreground");
    }
    return true;
}

void start()
{
    std::thread thread(loop);
    s_threadId = thread.get_id();
    thread.detach();
    SDL_AddEventWatch(lifecycleWatch, nullptr);
    SDL_Log("[GFX] OpenGL on a thread of its own (NFS_GL_THREAD=0 draws on the game thread)");
}

}

bool on()
{
    static const bool s_on = []() {
        const char* value = SDL_getenv("NFS_GL_THREAD");
        const bool on = !(value && SDL_strcmp(value, "0") == 0);
        if (on)
            start();
        return on;
    }();
    return s_on;
}

void post(Renderer* renderer, std::function<void()> work)
{
    if (!on())
    {
        renderer->setCurrent();
        work();
        renderer->clearCurrent();
        return;
    }
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        s_jobs.push_back(Job{renderer, std::move(work)});
    }
    s_wake.notify_one();
}

void finish()
{
    if (!on() || std::this_thread::get_id() == s_threadId)
        return;
    std::unique_lock<std::mutex> lock(s_mutex);
    s_idle.wait(lock, []() { return s_jobs.empty() && !s_busy; });
}

void release()
{
    if (!on())
        return;
    if (std::this_thread::get_id() == s_threadId)
    {
        if (s_current)
        {
            s_current->clearCurrent();
            s_current = nullptr;
        }
        return;
    }
    std::unique_lock<std::mutex> lock(s_mutex);
    s_releasing = true;
    s_wake.notify_one();
    s_idle.wait(lock, []() { return s_jobs.empty() && !s_busy && !s_releasing; });
}

unsigned long long presentPosted(int frames)
{
    if (!on())
        return 0;
    const Uint64 start = SDL_GetTicksNS();
    std::unique_lock<std::mutex> lock(s_mutex);
    ++s_presents;
    s_idle.wait(lock, [frames]() { return s_presents <= frames; });
    return SDL_GetTicksNS() - start;
}

void presentDone()
{
    if (!on())
        return;
    std::lock_guard<std::mutex> lock(s_mutex);
    --s_presents;
    s_idle.notify_all();
}

}
}
