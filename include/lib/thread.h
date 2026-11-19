#ifndef LIB_THREAD_H_
#define LIB_THREAD_H_

#include <lib/app.h>
#include <SDL3/SDL.h>

struct SDL_Thread;

namespace nfs
{

class Thread : public GenericResource
{
public:
    Thread();
    Thread(App* app, nfs::u32 method, nfs::u32 parameter, nfs::u32 flags);
    ~Thread();

    Thread* duplicate()
    {
        return new Thread(*this);
    }

    void resume();
    void terminate();
    nfs::u32 threadId() const;
    static void sleep(nfs::u32 milliseconds);
    static nfs::u32 currentThreadId();

private:
    Thread(const Thread& other);
    static int run(void* parameter);

private:
    struct Data
    {
        Data(App* app, nfs::u32 entryPoint, nfs::u32 parameter, nfs::u32 flags);
        App* m_app;
        nfs::u32      m_entryPoint;
        nfs::u32      m_parameter;
        nfs::u32      m_flags;
        nfs::u32      m_threadId;
    };

    SDL_AtomicInt*  m_refCount;
    SDL_Semaphore*  m_semaphore;
    Data            m_data;
    SDL_Thread*     m_thread;
    nfs::ThreadState        m_ts;
};

}

#endif

