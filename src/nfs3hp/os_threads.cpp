#include "os_threads.h"

#include <lib/event.h>
#include <lib/mutex.h>
#include <lib/thread.h>

#include <SDL3/SDL.h>
#include <vector>

namespace nfs3hp
{
namespace os
{

namespace locks
{

namespace
{

/* The port's lock a record of the game's holds at its start. */
nfs::Mutex* lockAt(App* app, nfs::u32 section)
{
    return app->getMemory<nfs::Mutex*>(section);
}

}  // namespace

void make(App* app, nfs::u32 section)
{
    nfs::Mutex* const lock = new nfs::Mutex("");
    app->getMemory<nfs::Mutex*>(section) = lock;
    app->allocateResource(lock);
}

void remove(App* app, nfs::u32 section)
{
    app->freeResource(lockAt(app, section)->getResourceIndex());
}

bool enter(App* app, nfs::ThreadState& ts, nfs::u32 section)
{
    nfs::Mutex* const lock = lockAt(app, section);
    /* A lock nobody else holds -- overwhelmingly the common case, since most of the game's locks
     * guard something brief -- is taken without letting the context go.  The context is shared
     * by every thread, so on a screen where more than one thread takes a lock at a high rate
     * (seen on the device: one thread alone at 18000+ lock/time/unlock triples a second while the
     * Camera/Graphics settings screens are open, others doing the same at the same time), letting
     * it go and taking it back on every call made the context's own mutex, not the lock, the one
     * every thread fought over.  The context is let go only when the lock is held by another
     * thread, which is the case that needs it: that thread may itself wait for the context, and
     * can give the lock back only once it has it. */
    if (lock->tryLock())
        return true;
    app->unlockContext(ts);
    lock->lock();
    app->lockContext(ts);
    return !ts.terminate;
}

bool leave(App* app, nfs::ThreadState& ts, nfs::u32 section)
{
    lockAt(app, section)->unlock();
    app->unlockContext(ts);
    app->lockContext(ts);
    return !ts.terminate;
}

std::optional<Handle> named(App* app, nfs::ThreadState& ts, const char* name, bool held)
{
    nfs::Mutex* const mutex = new nfs::Mutex(name);
    if (held)
    {
        app->unlockContext(ts);
        mutex->lock();
        app->lockContext(ts);
    }
    const Handle handle = app->allocateResource(mutex);
    if (ts.terminate)
        return std::nullopt;
    return handle;
}

}  // namespace locks

namespace threads
{

Started start(App* app, nfs::u32 entry, nfs::u32 parameter, nfs::u32 stackBytes, bool suspended)
{
    NFS2_ASSERT(stackBytes <= 1024 * 1024);
    nfs::Thread* const thread = new nfs::Thread(app, entry, parameter, suspended ? 0x4 : 0);
    if (!suspended)
        thread->resume();
    const Started started{app->allocateResource(thread), thread->threadId()};
    if (App::traceApi())
        SDL_Log("[API] thread started entry=0x%x param=0x%x suspended=%d -> h=%u tid=%u", unsigned(entry),
                unsigned(parameter), int(suspended), unsigned(started.handle), unsigned(started.id));
    return started;
}

void resume(App* app, Handle thread)
{
    dynamic_cast<nfs::Thread*>(app->getResource(thread))->resume();
}

bool stop(App* app, nfs::ThreadState& ts, Handle thread)
{
    nfs::Thread* const t = dynamic_cast<nfs::Thread*>(app->getResource(thread));
    app->unlockContext(ts);
    t->terminate();
    app->lockContext(ts);
    return !ts.terminate;
}

void exit(nfs::ThreadState& ts)
{
    ts.terminate = true;
}

Handle current(App* app)
{
    return app->allocateResource(new nfs::Thread());
}

nfs::u32 currentId(App* app, nfs::ThreadState& ts)
{
    return app->getMemory<nfs::u32>(ts.threadBlock + 0x4);
}

Handle duplicate(App* app, Handle thread)
{
    nfs::Thread* const t = dynamic_cast<nfs::Thread*>(app->getResource(thread));
    NFS2_ASSERT(t);
    return app->allocateResource(t->duplicate());
}

bool sleep(App* app, nfs::ThreadState& ts, nfs::u32 ms)
{
    app->unlockContext(ts);
    nfs::Thread::sleep(ms);
    app->lockContext(ts);
    return !ts.terminate;
}

nfs::u32 takeSlot()
{
    NFS2_ASSERT(false);
    return 0;
}

void giveBackSlot(nfs::u32 slot)
{
    NFS2_USE(slot);
    NFS2_ASSERT(false);
}

nfs::u32 slotValue(nfs::u32 slot)
{
    NFS2_USE(slot);
    NFS2_ASSERT(false);
    return 0;
}

bool setSlotValue(nfs::u32 slot, nfs::u32 value)
{
    NFS2_USE(slot);
    NFS2_USE(value);
    NFS2_ASSERT(false);
    return false;
}

}  // namespace threads

namespace events
{

namespace
{

nfs::Event* eventOf(App* app, Handle handle)
{
    return dynamic_cast<nfs::Event*>(app->getResource(handle));
}

/* src/lib's Event::wait answers with the place of the event set, or these. */
constexpr nfs::u32 kTimeUp = 0x102, kStopped = 0xffffffff;

}  // namespace

Handle make(App* app, bool manual, bool set, const char* name)
{
    return app->allocateResource(new nfs::Event(name, manual, set));
}

bool set(App* app, Handle event)
{
    nfs::Event* const e = eventOf(app, event);
    if (!e)
    {
        NFS2_ASSERT(false);
        return false;
    }
    e->set();
    return true;
}

bool reset(App* app, Handle event)
{
    NFS2_USE(app);
    NFS2_USE(event);
    NFS2_ASSERT(false);
    return false;
}

void pulse(App* app, Handle event)
{
    eventOf(app, event)->pulse();
}

std::optional<Wait> waitAny(App* app, nfs::ThreadState& ts, const Handle* handles, nfs::u32 count, nfs::u32 ms)
{
    std::vector<nfs::Event*> events(count);
    for (nfs::u32 i = 0; i < count; ++i)
    {
        events[i] = eventOf(app, handles[i]);
        if (!events[i])
        {
            NFS2_ASSERT(false);
            if (ts.terminate)
                return std::nullopt;
            return Wait{Ended::failed, 0};
        }
    }
    app->unlockContext(ts);
    const nfs::u32 result = nfs::Event::wait(ts, events.data(), events.data() + count, false, nfs::s32(ms));
    app->lockContext(ts);
    if (ts.terminate)
        return std::nullopt;
    if (result == kTimeUp)
        return Wait{Ended::timeUp, 0};
    if (result == kStopped)
        return Wait{Ended::failed, 0};
    return Wait{Ended::set, result};
}

std::optional<Wait> wait(App* app, nfs::ThreadState& ts, Handle event, nfs::u32 ms)
{
    return waitAny(app, ts, &event, 1, ms);
}

}  // namespace events

}  // namespace os
}  // namespace nfs3hp
