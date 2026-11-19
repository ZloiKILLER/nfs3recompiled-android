#ifndef LIB_MUTEX_H_
#define LIB_MUTEX_H_

#include <lib/app.h>
#include <string>

namespace nfs
{

class Mutex : public GenericResource
{
public:
    Mutex(const char* name);
    ~Mutex();

    void lock();
    /* Non-blocking: locks and returns true only if the mutex was free or
     * already owned by this thread; otherwise returns immediately without
     * taking the lock.  See locks::enter (src/nfs3hp/os_threads.cpp) for why this
     * exists: it lets the common, uncontended case skip releasing the guest
     * execution context altogether. */
    bool tryLock();
    void unlock();
    /* Owner only: lets go of every level of a recursive hold at once and says
     * how many there were, for a yield to take them all back
     * (App::yieldContext). */
    nfs::u32 unlockAll();

    nfs::u32 getResourceIndex() const { return m_resourceIndex; }

private:
    SDL_Mutex*  m_mutex;
    std::string m_name;
    nfs::u32  m_owner;
    nfs::u32  m_count;
};

}

#endif

