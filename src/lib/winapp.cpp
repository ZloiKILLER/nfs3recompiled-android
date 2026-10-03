#include <lib/winapp.h>
#include <lib/mutex.h>
#include <lib/event.h>
#include <lib/library.h>
#include <lib/registry.h>
#include <winapi/types.h>
#include <SDL3/SDL.h>
#include <x86.h>

#include <winapi/dsetup.h>
#include <winapi/ddraw.h>
#include <winapi/glide2x.h>
#include <algorithm>
#include <set>

namespace win32
{

int WinApplication::s_traceApi = -1;

bool WinApplication::readTraceApi()
{
    // read once, on first use; SDL_getenv is safe before SDL_Init
    const char* v = SDL_getenv("NFS_TRACE_API");
    s_traceApi = v && *v && *v != '0' ? 1 : 0;
    return s_traceApi != 0;
}

/* traceNewCalls: 2 in s_traceApi, and the thread and target pairs seen. */
static SDL_Mutex* s_seenCallsLock = nullptr;
static std::set<std::uint64_t>* s_seenCalls = nullptr;

void WinApplication::traceNewCalls()
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

void WinApplication::traceCall(const Method& method, x86::reg32 address)
{
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

static x86::reg32 s_resourceIndex;
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

void GenericResource::setResourceIndex(x86::reg32 resourceIndex)
{
    NFS2_ASSERT(!resourceIndex || !m_resourceIndex);
    m_resourceIndex = resourceIndex;
}

x86::reg32 GenericResource::getResourceIndex() const
{
    return m_resourceIndex;
}


WinApplication::WinApplication(const char* appName, x86::reg32 baseAddress, const std::vector<Section>& sections)
    :   m_memory(MemMap::init(baseAddress, sections))
    ,   m_appName(new MemMap(4096))
    ,   m_appNameW(new MemMap(4096))
    ,   m_env(new MemMap(4096))
    ,   m_executionContext(new Mutex("ExecutionContext"))
    ,   m_resourceContext(new Mutex("Resource"))
    ,   m_cpu{}
{
    /* Place 0 of the method list stands for "no method" in the page table. */
    m_methodList.emplace_back();
    dsetup::s_dSetupRegistry->registerSymbols(this);
    ddraw::s_dDrawRegistry->registerSymbols(this);
    glide2x::s_glide2xRegistry->registerSymbols(this);
    Event::init();
    strcpy(&getMemory<char>(m_appName->getBlockStart()), appName);
    getMemory<char>(m_env->getBlockStart()) = 0;
    WCHAR* wchar = &getMemory<WCHAR>(m_appNameW->getBlockStart());
    *wchar++ = 0xfffe;
    for (const char* appNameW = appName; *appNameW; ++wchar, ++appNameW)
    {
        *wchar = WCHAR(*appNameW);
    }
    *wchar = 0;
    allocateResourceFixed(new RegistryKey(), HKEY_CLASSES_ROOT);
    allocateResourceFixed(new RegistryKey(), HKEY_CURRENT_USER);
    allocateResourceFixed(new RegistryKey(), HKEY_LOCAL_MACHINE);
    allocateResourceFixed(new RegistryKey(), HKEY_USERS);
    allocateResourceFixed(new RegistryKey(), HKEY_PERFORMANCE_DATA);
    allocateResourceFixed(new RegistryKey(), HKEY_PERFORMANCE_TEXT);
    allocateResourceFixed(new RegistryKey(), HKEY_PERFORMANCE_NLSTEXT);
    allocateResourceFixed(new RegistryKey(), HKEY_CURRENT_CONFIG);
    allocateResourceFixed(new RegistryKey(), HKEY_DYN_DATA);
}

WinApplication::~WinApplication()
{
    delete m_appNameW;
    delete m_appName;
    m_resourceContext->lock();
    for (std::map<x86::reg32, GenericResource*>::const_iterator it = m_resources.begin();
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

void WinApplication::addRegistryKey(x86::reg32 root, const char* keyname, const char* valuename, RegistryValue* value)
{
    RegistryKey* key = dynamic_cast<RegistryKey*>(getResource(root));
    NFS2_ASSERT(key);
    const char* subk = keyname;
    while (*subk)
    {
        const char* subkEnd;
        for (subkEnd = subk; *subkEnd && *subkEnd != '\\'; ++subkEnd)
            /* nothing */;
        std::string subkey = std::string(subk, subkEnd);
        RegistryKey* newkey = key->getChild(subkey);
        if (!newkey)
        {
            newkey = new RegistryKey();
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

x86::reg32 WinApplication::allocateResourceFixed(GenericResource *resource, x86::reg32 fixedHandle)
{
    NFS2_ASSERT((fixedHandle < 0x20000) || (fixedHandle > 0x20000 + 0x30000));
    m_resourceContext->lock();
    resource->setResourceIndex(fixedHandle);
    bool inserted = m_resources.insert(std::make_pair(fixedHandle, resource)).second;
    NFS2_ASSERT(inserted);
    m_resourceContext->unlock();
    return fixedHandle;
}

x86::reg32 WinApplication::allocateResource(GenericResource *resource)
{
    m_resourceContext->lock();
    x86::reg32 resourceIndex = ++s_resourceIndex + 0x20000;
    resource->setResourceIndex(resourceIndex);
    bool inserted = m_resources.insert(std::make_pair(resourceIndex, resource)).second;
    NFS2_ASSERT(inserted);
    m_resourceContext->unlock();
    return resourceIndex;
}

GenericResource* WinApplication::getResource(x86::reg32 resource)
{
    GenericResource* result = nullptr;
    m_resourceContext->lock();
    std::map<x86::reg32, GenericResource*>::iterator it = m_resources.find(resource);
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

void WinApplication::freeResource(x86::reg32 resource)
{
    if (resource != 0xffffffff)
    {
        m_resourceContext->lock();
        std::map<x86::reg32, GenericResource*>::iterator it = m_resources.find(resource);
        NFS2_ASSERT(it != m_resources.end());
        it->second->setResourceIndex(0);
        delete it->second;
        m_resources.erase(it);
        m_resourceContext->unlock();
    }
}

void WinApplication::registerMethod(x86::reg32 pointer, Method method)
{
    NFS2_ASSERT(m_methods.find(pointer-0x400000) == m_methods.end());
    m_methods[pointer-0x400000] = method;
    /* And into the page table dynamic_call reads, while there is room in its
     * 16-bit places; one past that is found in the map instead. */
    const x86::reg32 offset = pointer - kMethodBase;
    if (offset < kMethodSpan && m_methodList.size() <= 0xffff)
    {
        std::uint16_t*& page = m_methodPages[offset >> kMethodPageBits];
        if (!page)
            page = new std::uint16_t[kMethodPageMask + 1]();
        page[offset & kMethodPageMask] = std::uint16_t(m_methodList.size());
        m_methodList.push_back(method);
    }
}

void WinApplication::dynamicCallElsewhere(x86::reg32 address, x86::CPU& cpu)
{
    std::unordered_map<x86::reg32, Method>::const_iterator it = m_methods.find(address - 0x400000);
    if (it == m_methods.end())
    {
        /* Running the miss used to read the method pointer straight out of
         * end(), which libc++ represents as a null node -- a SIGSEGV at offset
         * 0x30 naming neither the caller nor the address it wanted.  Skipping
         * the call leaves the guest registers as the caller left them, which
         * is survivable; the crash was not. */
        reportMissingMethod(address);
        return;
    }
    const Method& m = it->second;
    if (traceApi())
        traceCall(m, address);
    m(this, cpu);
}

void WinApplication::reportMissingMethod(x86::reg32 address)
{
    /* One line per address, and never more.  A missed target usually sits in a
     * path the game retries every frame, and the flood would push the first --
     * the only interesting -- occurrence out of the log buffer.  Guest threads
     * make these calls, so the seen set needs a lock of its own. */
    static SDL_Mutex* const mutex = SDL_CreateMutex();
    static std::set<x86::reg32> reported;

    SDL_LockMutex(mutex);
    const bool first = reported.insert(address).second;
    SDL_UnlockMutex(mutex);

    if (first)
    {
        SDL_Log("[API] no method registered for 0x%08x -- indirect call skipped",
                unsigned(address));
    }
}

int WinApplication::runThread(x86::CPU& cpu, x86::reg32 entryPoint, x86::reg32 parameter, bool threadLock)
{
    if (!m_cpu.terminate)
    {
        win32::MemMap threadStorage(4096);
        win32::MemMap threadStack(1024 * 1024);
        return runOn(cpu, threadStorage, threadStack, entryPoint, parameter, threadLock);
    }
    else
    {
        return 0;
    }
}

int WinApplication::runCallback(x86::CPU& cpu, x86::reg32 entryPoint, x86::reg32 parameter)
{
    /* Timer callbacks come back on SDL's timer thread about 126 times a second.
     * Mapping a fresh 1 MB guest stack and thread block for each and unmapping
     * it again took half of that thread's time.  Calls never overlap on one
     * host thread, so each thread keeps its pair for good. */
    thread_local win32::MemMap* threadStorage = nullptr;
    thread_local win32::MemMap* threadStack = nullptr;
    if (m_cpu.terminate)
    {
        return 0;
    }
    if (!threadStack)
    {
        threadStorage = new win32::MemMap(4096);
        threadStack = new win32::MemMap(1024 * 1024);
    }
    return runOn(cpu, *threadStorage, *threadStack, entryPoint, parameter, true);
}

int WinApplication::runOn(x86::CPU& cpu, const MemMap& threadStorage, const MemMap& threadStack,
                          x86::reg32 entryPoint, x86::reg32 parameter, bool threadLock)
{
    cpu.init(threadStorage.getBlockStart(), entryPoint);
    getMemory<Mutex*>(cpu.efs + 8) = threadLock ? m_executionContext : nullptr;
    LockContext ctx(*this, cpu);
    cpu.esp = threadStack.getBlockStart() + threadStack.getBlockSize() - 8;
    getMemory<x86::reg32>(cpu.esp + 4) = parameter;
    getMemory<x86::reg32>(cpu.esp) = cpu.ip;
    dynamic_call(entryPoint, cpu);
    return int(cpu.eax);
}

void WinApplication::lockContext(const x86::CPU& cpu)
{
    Mutex* executionContext = getMemory<Mutex*>(cpu.efs + 8);
    NFS2_ASSERT(executionContext);
    if (executionContext)
    {
        acquireContext(executionContext, 1);
    }
}

/* Takes the context `depth` levels deep.  A thread that has to wait for it is
 * counted while it waits, so that the thread running meets a safepoint and
 * hands it over (yieldContext). */
void WinApplication::acquireContext(Mutex* executionContext, x86::reg32 depth)
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
    for (x86::reg32 i = 1; i < depth; ++i)
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
void WinApplication::yieldContext(const x86::CPU& cpu)
{
    static constexpr std::uint64_t kSliceNs = 2000000;
    if ((++m_yieldPolls & 63) != 0)
    {
        return;
    }
    Mutex* executionContext = getMemory<Mutex*>(cpu.efs + 8);
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
    const x86::reg32 depth = executionContext->unlockAll();
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

void WinApplication::unlockContext(const x86::CPU& cpu)
{
    Mutex* executionContext = getMemory<Mutex*>(cpu.efs + 8);
    NFS2_ASSERT(executionContext);
    if (executionContext)
    {
        executionContext->unlock();
    }
}

void WinApplication::unmarkContext(const x86::CPU& cpu)
{
    Mutex* executionContext = getMemory<Mutex*>(cpu.efs + 8);
    if (executionContext)
    {
        executionContext->unlock();
        getMemory<Mutex*>(cpu.efs + 8) = nullptr;
    }
    else
    {
        NFS2_ASSERT(false);
    }
}

void WinApplication::terminate()
{
    m_cpu.terminate = true;
}

}
