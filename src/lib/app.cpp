#include <lib/app.h>
#include <lib/mutex.h>
#include <lib/event.h>
#include <lib/library.h>
#include <lib/settings.h>
#include <SDL3/SDL.h>
#include <game_types.h>

#include <algorithm>
#include <set>

namespace nfs
{

int App::s_traceApi = -1;

bool App::readTraceApi()
{
    // read once, on first use; SDL_getenv is safe before SDL_Init
    const char* v = SDL_getenv("NFS_TRACE_API");
    s_traceApi = v && *v && *v != '0' ? 1 : 0;
    return s_traceApi != 0;
}

/* traceNewCalls: 2 in s_traceApi, and the thread and target pairs seen. */
static SDL_Mutex* s_seenCallsLock = nullptr;
static std::set<std::uint64_t>* s_seenCalls = nullptr;

void App::traceNewCalls()
{
    if (!s_seenCallsLock)
    {
        s_seenCallsLock = SDL_CreateMutex();
        s_seenCalls = new std::set<std::uint64_t>();
    }
    SDL_LockMutex(s_seenCallsLock);
    s_seenCalls->clear();
    SDL_UnlockMutex(s_seenCallsLock);
    s_traceApi = 2;
}

void App::traceCall(nfs::u32 address)
{
    const auto it = m_methods.find(address - 0x400000);
    static const Method none;
    const Method& method = it != m_methods.end() ? it->second : none;
    const unsigned thread = unsigned(SDL_GetCurrentThreadID());
    if (s_traceApi != 2)
    {
        SDL_Log("[API] t%u %s", thread, method.name.c_str());
        return;
    }
    SDL_LockMutex(s_seenCallsLock);
    const bool first = s_seenCalls->insert(std::uint64_t(thread) << 32 | address).second;
    SDL_UnlockMutex(s_seenCallsLock);
    if (first)
        SDL_Log("[CALL] t%u 0x%08x %s", thread, unsigned(address), method.name.c_str());
}

static nfs::u32 s_resourceIndex;
static SDL_AtomicInt s_resourceCount;

GenericResource::GenericResource()
    :   m_resourceIndex(0)
{
    SDL_AddAtomicInt(&s_resourceCount, 1);
}

GenericResource::~GenericResource()
{
    NFS2_ASSERT(m_resourceIndex == 0);
    SDL_AddAtomicInt(&s_resourceCount, -1);
}

void GenericResource::setResourceIndex(nfs::u32 resourceIndex)
{
    NFS2_ASSERT(!resourceIndex || !m_resourceIndex);
    m_resourceIndex = resourceIndex;
}

nfs::u32 GenericResource::getResourceIndex() const
{
    return m_resourceIndex;
}


App::App(const char* appName, nfs::u32 baseAddress, const std::vector<Section>& sections)
    :   m_memory(MemMap::init(baseAddress, sections))
    ,   m_appName(new MemMap(4096))
    ,   m_appNameW(new MemMap(4096))
    ,   m_env(new MemMap(4096))
    ,   m_executionContext(new Mutex("ExecutionContext"))
    ,   m_resourceContext(new Mutex("Resource"))
    ,   m_ts{}
{
    /* Place 0 of the method list stands for "no method" in the page table. */
    m_methodList.emplace_back();
    Event::init();
    strcpy(&getMemory<char>(m_appName->getBlockStart()), appName);
    getMemory<char>(m_env->getBlockStart()) = 0;
    nfs::u16* wchar = &getMemory<nfs::u16>(m_appNameW->getBlockStart());
    *wchar++ = 0xfffe;
    for (const char* appNameW = appName; *appNameW; ++wchar, ++appNameW)
    {
        *wchar = nfs::u16(*appNameW);
    }
    *wchar = 0;
    for (SettingsRoot root : { SettingsRoot::classes, SettingsRoot::currentUser, SettingsRoot::localMachine })
        allocateResourceFixed(new SettingsKey(), nfs::u32(root));
}

App::~App()
{
    delete m_appNameW;
    delete m_appName;
    m_resourceContext->lock();
    for (std::map<nfs::u32, GenericResource*>::const_iterator it = m_resources.begin();
        it != m_resources.end();
        it = m_resources.erase(it))
    {
        it->second->setResourceIndex(0);
        delete it->second;
    }
    m_resourceContext->unlock();
    Event::fini();
    MemMap::fini();
    delete m_resourceContext;
    delete m_executionContext;
    for (std::uint16_t* page : m_methodPages)
        delete[] page;
    NFS2_ASSERT(SDL_GetAtomicInt(&s_resourceCount) == 0);
}

void App::addSetting(SettingsRoot root, const char* keyname, const char* valuename, SettingValue* value)
{
    SettingsKey* key = dynamic_cast<SettingsKey*>(getResource(nfs::u32(root)));
    NFS2_ASSERT(key);
    const char* subk = keyname;
    while (*subk)
    {
        const char* subkEnd;
        for (subkEnd = subk; *subkEnd && *subkEnd != '\\'; ++subkEnd)
            /* nothing */;
        std::string subkey = std::string(subk, subkEnd);
        SettingsKey* newkey = key->getChild(subkey);
        if (!newkey)
        {
            newkey = new SettingsKey();
            allocateResource(newkey);
            key->addChild(subkey, newkey);
        }
        key = newkey;
        subk = subkEnd;
        if (*subk) subk++;
    }
    allocateResource(value);
    key->addValue(valuename, value);
}

nfs::u32 App::allocateResourceFixed(GenericResource *resource, nfs::u32 fixedHandle)
{
    NFS2_ASSERT((fixedHandle < 0x20000) || (fixedHandle > 0x20000 + 0x30000));
    m_resourceContext->lock();
    resource->setResourceIndex(fixedHandle);
    bool inserted = m_resources.insert(std::make_pair(fixedHandle, resource)).second;
    NFS2_ASSERT(inserted);
    m_resourceContext->unlock();
    return fixedHandle;
}

nfs::u32 App::allocateResource(GenericResource *resource)
{
    m_resourceContext->lock();
    nfs::u32 resourceIndex = ++s_resourceIndex + 0x20000;
    resource->setResourceIndex(resourceIndex);
    bool inserted = m_resources.insert(std::make_pair(resourceIndex, resource)).second;
    NFS2_ASSERT(inserted);
    m_resourceContext->unlock();
    return resourceIndex;
}

GenericResource* App::getResource(nfs::u32 resource)
{
    GenericResource* result = nullptr;
    m_resourceContext->lock();
    std::map<nfs::u32, GenericResource*>::iterator it = m_resources.find(resource);
    if (it != m_resources.end())
    {
        result = it->second;
    }
    else
    {
        result = nullptr;
    }
    m_resourceContext->unlock();
    return result;
}

void App::freeResource(nfs::u32 resource)
{
    if (resource != 0xffffffff)
    {
        m_resourceContext->lock();
        std::map<nfs::u32, GenericResource*>::iterator it = m_resources.find(resource);
        NFS2_ASSERT(it != m_resources.end());
        it->second->setResourceIndex(0);
        delete it->second;
        m_resources.erase(it);
        m_resourceContext->unlock();
    }
}

void App::registerMethod(nfs::u32 pointer, Method method)
{
    NFS2_ASSERT(m_methods.find(pointer-0x400000) == m_methods.end());
    m_methods[pointer-0x400000] = method;
    /* And into the page table callAddress reads, while there is room in its
     * 16-bit places; one past that is found in the map instead. */
    const nfs::u32 offset = pointer - kMethodBase;
    if (offset < kMethodSpan && m_methodList.size() <= 0xffff)
    {
        std::uint16_t*& page = m_methodPages[offset >> kMethodPageBits];
        if (!page)
            page = new std::uint16_t[kMethodPageMask + 1]();
        page[offset & kMethodPageMask] = std::uint16_t(m_methodList.size());
        m_methodList.push_back(method);
    }
}

void App::replaceMethod(nfs::u32 pointer, Method method)
{
    NFS2_ASSERT(m_methods.find(pointer-0x400000) != m_methods.end());
    m_methods.erase(pointer-0x400000);
    registerMethod(pointer, std::move(method));
}

ThunkPtr App::thunkElsewhere(nfs::u32 address) const
{
    const auto it = m_methods.find(address - 0x400000);
    return it == m_methods.end() ? nullptr : it->second.thunk;
}

void App::reportMissingMethod(nfs::u32 address)
{
    /* One line per address, and never more.  A missed target usually sits in a
     * path the game retries every frame, and the flood would push the first --
     * the only interesting -- occurrence out of the log buffer.  Guest threads
     * make these calls, so the seen set needs a lock of its own. */
    static SDL_Mutex* const mutex = SDL_CreateMutex();
    static std::set<nfs::u32> reported;

    SDL_LockMutex(mutex);
    const bool first = reported.insert(address).second;
    SDL_UnlockMutex(mutex);

    if (first)
    {
        SDL_Log("[API] no method registered for 0x%08x -- indirect call skipped",
                unsigned(address));
    }
}

int App::runThread(nfs::ThreadState& ts, nfs::u32 entryPoint, nfs::u32 parameter, bool threadLock)
{
    if (!m_ts.terminate)
    {
        nfs::MemMap threadStorage(4096);
        nfs::MemMap threadStack(1024 * 1024);
        return runOn(ts, threadStorage, threadStack, entryPoint, parameter, threadLock);
    }
    else
    {
        return 0;
    }
}

int App::runCallback(nfs::ThreadState& ts, nfs::u32 entryPoint, nfs::u32 parameter)
{
    /* Timer callbacks come back on SDL's timer thread about 126 times a second.
     * Mapping a fresh 1 MB guest stack and thread block for each and unmapping
     * it again took half of that thread's time.  Calls never overlap on one
     * host thread, so each thread keeps its pair for good. */
    thread_local nfs::MemMap* threadStorage = nullptr;
    thread_local nfs::MemMap* threadStack = nullptr;
    if (m_ts.terminate)
    {
        return 0;
    }
    if (!threadStack)
    {
        threadStorage = new nfs::MemMap(4096);
        threadStack = new nfs::MemMap(1024 * 1024);
    }
    return runOn(ts, *threadStorage, *threadStack, entryPoint, parameter, true);
}

int App::runOn(nfs::ThreadState& ts, const MemMap& threadStorage, const MemMap& threadStack,
                          nfs::u32 entryPoint, nfs::u32 parameter, bool threadLock)
{
    ts.init(threadStorage.getBlockStart());
    getMemory<Mutex*>(ts.threadBlock + 8) = threadLock ? m_executionContext : nullptr;
    LockContext ctx(*this, ts);
    ts.sp = threadStack.getBlockStart() + threadStack.getBlockSize() - 8;
    getMemory<nfs::u32>(ts.sp + 4) = parameter;
    getMemory<nfs::u32>(ts.sp) = entryPoint;  // the return address's slot
    nfs::Args args;
    return int(callAddress(entryPoint, ts, args));
}

/* KeepContext's count: while it is above zero, this thread neither hands the
 * context over at a safepoint nor lets it go in an API call that waits (Sleep,
 * a frame's flip...).  A wait for another guest thread inside a check would
 * hang; the natives checked draw and compute, and do not. */
static thread_local unsigned t_keepContext = 0;

App::KeepContext::KeepContext()
{
    ++t_keepContext;
}

App::KeepContext::~KeepContext()
{
    --t_keepContext;
}

void App::lockContext(const nfs::ThreadState& ts)
{
    if (t_keepContext)
    {
        return;
    }
    Mutex* executionContext = getMemory<Mutex*>(ts.threadBlock + 8);
    NFS2_ASSERT(executionContext);
    if (executionContext)
    {
        acquireContext(executionContext, 1);
    }
}

/* Takes the context `depth` levels deep.  A thread that has to wait for it is
 * counted while it waits, so that the thread running meets a safepoint and
 * hands it over (yieldContext). */
void App::acquireContext(Mutex* executionContext, nfs::u32 depth)
{
    if (!executionContext->tryLock())
    {
        if (m_contextWaiters.fetch_add(1, std::memory_order_relaxed) == 0)
        {
            m_contextWantedSince.store(SDL_GetTicksNS(), std::memory_order_relaxed);
        }
        executionContext->lock();
        m_contextWaiters.fetch_sub(1, std::memory_order_relaxed);
        m_contextHandoffs.fetch_add(1, std::memory_order_release);
    }
    for (nfs::u32 i = 1; i < depth; ++i)
    {
        executionContext->lock();
    }
}

/* A safepoint found a thread waiting for the context (contextWanted).  Once
 * that thread has waited out a slice, the context goes to it: every level of
 * this thread's hold let go, a moment for the waiter to wake and take it --
 * a mutex let go and taken straight back would, more often than not, go to
 * the same thread again -- then this thread queues for it like any other.
 * The slice is 2 ms, a tenth of Windows 98's: the sound's timer and mixer come
 * back quickly, and a thread that computes flat out still hands over rarely.
 * The clock is read on one check in 64 only, to keep a hot loop cheap while a
 * waiter's slice runs. */
void App::yieldContext(const nfs::ThreadState& ts)
{
    static constexpr std::uint64_t kSliceNs = 2000000;
    if ((++m_yieldPolls & 63) != 0 || t_keepContext)
    {
        return;
    }
    Mutex* executionContext = getMemory<Mutex*>(ts.threadBlock + 8);
    if (!executionContext || !contextWanted()
        || SDL_GetTicksNS() - m_contextWantedSince.load(std::memory_order_relaxed) < kSliceNs)
    {
        return;
    }
    /* How often this happens, on the 1st, 2nd, 4th, 8th... time: rare in play,
     * and each line says how long the waiter had waited. */
    static unsigned s_yields = 0;
    ++s_yields;
    if ((s_yields & (s_yields - 1)) == 0)
    {
        SDL_Log("[SAFEPOINT] handoff %u, the waiter waited %.1f ms", s_yields,
                double(SDL_GetTicksNS() - m_contextWantedSince.load(std::memory_order_relaxed)) / 1e6);
    }
    const unsigned handoffs = m_contextHandoffs.load(std::memory_order_acquire);
    const nfs::u32 depth = executionContext->unlockAll();
    for (int i = 0; i < 200 && contextWanted()
                    && m_contextHandoffs.load(std::memory_order_acquire) == handoffs; ++i)
    {
        if (i < 20)
        {
            SDL_CPUPauseInstruction();
        }
        else
        {
            SDL_DelayNS(20000);
        }
    }
    acquireContext(executionContext, depth);
}

void App::unlockContext(const nfs::ThreadState& ts)
{
    if (t_keepContext)
    {
        return;
    }
    Mutex* executionContext = getMemory<Mutex*>(ts.threadBlock + 8);
    NFS2_ASSERT(executionContext);
    if (executionContext)
    {
        executionContext->unlock();
    }
}

void App::unmarkContext(const nfs::ThreadState& ts)
{
    Mutex* executionContext = getMemory<Mutex*>(ts.threadBlock + 8);
    if (executionContext)
    {
        executionContext->unlock();
        getMemory<Mutex*>(ts.threadBlock + 8) = nullptr;
    }
    else
    {
        NFS2_ASSERT(false);
    }
}

void App::terminate()
{
    m_ts.terminate = true;
}

}
