#ifndef LIB_MUTEX_H_
#define LIB_MUTEX_H_

#include <lib/winapp.h>
#include <string>

namespace win32
{

class Mutex : public GenericResource
{
public:
    Mutex(const char* name);
    ~Mutex();

    void lock();
    /* Non-blocking: locks and returns true only if the mutex was free or
     * already owned by this thread; otherwise returns immediately without
     * taking the lock.  See EnterCriticalSection (kernel32.cpp) for why this
     * exists: it lets the common, uncontended case skip releasing the guest
     * execution context altogether. */
    bool tryLock();
    void unlock();

    x86::reg32 getResourceIndex() const { return m_resourceIndex; }

private:
    SDL_Mutex*  m_mutex;
    std::string m_name;
    x86::reg32  m_owner;
    x86::reg32  m_count;
};

}

#endif

