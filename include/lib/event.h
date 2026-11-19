#ifndef LIB_EVENT_H_
#define LIB_EVENT_H_
#include <lib/app.h>
#include <string>
#include <vector>

namespace nfs
{

class Event : public GenericResource
{
public:
    Event(const char* name, bool manualReset, bool initialState);
    ~Event();

    void set();
    nfs::u32 pulse();
    static nfs::u32 wait(nfs::ThreadState& ts, Event** begin, Event** end, bool waitAll, nfs::s32 timeout);
    static void broadcastTerminate();

    static void init();
    static void fini();

private:
    std::string             m_name;
    std::vector<nfs::u32> m_waitingThreads;
    bool                    m_manualReset;
    bool                    m_isSet;
    bool                    m_pulse;
    unsigned                m_id;
};

}

#endif /*!EVENT_H_*/
