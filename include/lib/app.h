#ifndef LIB_APP_H_
#define LIB_APP_H_

#include    <game_types.h>
#include    <game_thread.h>
#include    <game_float.h>
#include    <lib/memmap.h>
#include    <atomic>
#include    <cstdint>
#include    <map>
#include    <string>
#include    <unordered_map>
#include    <vector>
#include    <SDL3/SDL.h>

namespace nfs
{

class App;
class Mutex;
class MemMap;
class SettingValue;
enum class SettingsRoot : nfs::u32;

/* A function the game calls through a pointer (its tables, callbacks, threads, imports): its
 * arguments in `args` by Watcom's convention, its stack words where ts.sp points; its eax
 * returned, what else it leaves for the caller in `args`. */
typedef nfs::u32 (*ThunkPtr)(App* application, nfs::ThreadState& ts, nfs::Args& args);

struct Method
{
    std::string name;
    ThunkPtr thunk = nullptr;

    Method() = default;
    Method(std::string n, ThunkPtr t) : name(std::move(n)), thunk(t) {}
};

class GenericResource
{
    friend class App;
public:
    virtual ~GenericResource();
    nfs::u32 getResourceIndex() const;

protected:
    GenericResource();

    void setResourceIndex(nfs::u32 resourceIndex);

protected:
    nfs::u32  m_resourceIndex;
};

template< typename T >
struct MemoryAccessor
{
    friend class App;
private:
    MemoryAccessor(nfs::u8* address) : address(address) {}
    nfs::u8* address;

public:
    MemoryAccessor(const MemoryAccessor& other) = default;
    MemoryAccessor& operator=(const MemoryAccessor& other)
    {
        if (this != &other)
        {
            T value = other;
            memcpy(address, &value, sizeof(T));
        }
        return *this;
    }
    operator T() const { T result; memcpy(&result, address, sizeof(T)); return result; }
    MemoryAccessor& operator=(T value) { memcpy(address, &value, sizeof(T)); return *this; }
    T operator-=(T value) { T result = *this; result -= value; memcpy(address, &result, sizeof(T)); return result; }
    T operator+=(T value) { T result = *this; result += value; memcpy(address, &result, sizeof(T)); return result; }
    T operator|=(T value) { T result = *this; result |= value; memcpy(address, &result, sizeof(T)); return result; }
    T operator&=(T value) { T result = *this; result &= value; memcpy(address, &result, sizeof(T)); return result; }
    T operator^=(T value) { T result = *this; result ^= value; memcpy(address, &result, sizeof(T)); return result; }
    T operator>>=(T value) { T result = *this; result >>= value; memcpy(address, &result, sizeof(T)); return result; }
    T operator<<=(T value) { T result = *this; result <<= value; memcpy(address, &result, sizeof(T)); return result; }
    T operator++(int) { T result = *this; T newValue = result; newValue++; memcpy(address, &newValue, sizeof(T)); return result; }
    T operator--(int) { T result = *this; T newValue = result; newValue--; memcpy(address, &newValue, sizeof(T)); return result; }
    T operator++() { T result = *this; T newValue = result; newValue++; memcpy(address, &newValue, sizeof(T)); return newValue; }
    T operator--() { T result = *this; T newValue = result; newValue--; memcpy(address, &newValue, sizeof(T)); return newValue; }
    T* operator&() { return reinterpret_cast<T*>(address); }
};

template<>
struct MemoryAccessor<nfs::Float80>
{
    friend class App;
private:
    MemoryAccessor(nfs::u8* address) : address(address) {}
    nfs::u8* address;

public:
    MemoryAccessor(const MemoryAccessor& other) = default;
    MemoryAccessor& operator=(const MemoryAccessor& other) = delete;
    operator nfs::Float() const { nfs::Float80Data result; memcpy(&result, address, sizeof(nfs::Float80Data)); return double(nfs::Float80(result)); }
    MemoryAccessor& operator=(nfs::Float value) { nfs::Float80 f80value(value); memcpy(address, &f80value.data, sizeof(nfs::Float80Data)); return *this; }
    operator nfs::Float80() const { nfs::Float80Data result; memcpy(&result, address, sizeof(nfs::Float80Data)); return nfs::Float80(result); }
    MemoryAccessor& operator=(nfs::Float80 value) { memcpy(address, &value.data, sizeof(nfs::Float80Data)); return *this; }
};

template<>
struct MemoryAccessor<void>
{
    friend class App;
private:
    MemoryAccessor(nfs::u8* address) : address(address) {}
    nfs::u8* address;

public:
    MemoryAccessor(const MemoryAccessor& other) = default;
    MemoryAccessor& operator=(const MemoryAccessor& other) = delete;
    void* operator&() { return reinterpret_cast<void*>(address); }
};

template<>
struct MemoryAccessor<const void>
{
    friend class App;
private:
    MemoryAccessor(nfs::u8* address) : address(address) {}
    nfs::u8* address;

public:
    MemoryAccessor(const MemoryAccessor& other) = default;
    MemoryAccessor& operator=(const MemoryAccessor& other) = delete;
    const void* operator&() { return reinterpret_cast<const void*>(address); }
};

class App
{
public:
    GenericResource* getResource(nfs::u32 resource);
    void freeResource(nfs::u32 resource);
    nfs::u32 allocateResource(GenericResource* resource);
    nfs::u32 getAppName() const { return m_appName->getBlockStart(); }
    nfs::u32 getAppNameW() const { return m_appNameW->getBlockStart(); }
    nfs::u32 getEnv() const { return m_env->getBlockStart(); }
    char* getAppNameRaw() { return &getMemory<char>(m_appName->getBlockStart()); }

    /* The setting `name` under `key` of `root` (its parts split by \), with `value`: the port's
     * table of what the game looks up (lib/settings.h). */
    void addSetting(SettingsRoot root, const char* key, const char* name, SettingValue* value);

    template< typename T >
    inline MemoryAccessor<T> getMemory(nfs::u32 address) { return MemoryAccessor<T>{m_memory + address}; }

    /* Reverse of getMemory(): turn a host pointer that lives inside the guest
     * address space back into the guest address the game itself sees.  Only
     * used by NFS_TRACE_API logging, where knowing *which* guest buffer a
     * string came from is what separates "the game handed us an empty name"
     * from "our layer read the wrong place". */
    inline nfs::u32 guestAddress(const void* pointer) const
    {
        return pointer ? nfs::u32(static_cast<const nfs::u8*>(pointer) - m_memory) : 0;
    }

    void registerMethod(nfs::u32 pointer, Method method);
    /* A method registered already, put another in its place. */
    void replaceMethod(nfs::u32 pointer, Method method);

    int runThread(nfs::ThreadState& ts, nfs::u32 entryPoint, nfs::u32 parameter = 0, bool threadLock = true);
    /* runThread for callbacks a host thread makes over and over (the timer):
     * the guest stack and thread block stay mapped between calls. */
    int runCallback(nfs::ThreadState& ts, nfs::u32 entryPoint, nfs::u32 parameter);

    void lockContext(const nfs::ThreadState& ts);
    void unlockContext(const nfs::ThreadState& ts);
    void unmarkContext(const nfs::ThreadState& ts);

    /* Safepoints.  One guest thread runs at a time, holding the execution
     * context, and it only let go of it inside some API calls; a loop that
     * waits for another guest thread without calling one of those -- the
     * showcase's exit waiting for the sound thread to stop the narration
     * (sub_4e7b28) -- could keep that thread out for good, where Windows 98
     * would have taken the processor away from it at the end of its time slice.
     * Such a loop has a safepoint at its head: whether a thread is
     * waiting for the context, a load and a branch; and when one is, and has
     * waited longer than a slice, yieldContext hands the context over. */
    inline bool contextWanted() const
    {
        return m_contextWaiters.load(std::memory_order_relaxed) != 0;
    }
    void yieldContext(const nfs::ThreadState& ts);
    /* While one is alive, this thread keeps the context, at its safepoints and
     * in the API calls that would let it go while they wait. */
    struct KeepContext
    {
        KeepContext();
        ~KeepContext();
        KeepContext(const KeepContext&) = delete;
        KeepContext& operator=(const KeepContext&) = delete;
    };

    void terminate();

public:
    // Set NFS_TRACE_API=1 in the environment to log every call through a pointer by name.
    // Compiled in unconditionally so it can be toggled without a full rebuild of
    // the ~60 MB of the game's code that includes this header.  Read once, on
    // first use; after that a load and a compare, since app->callAddress asks it on
    // every call through a pointer.
    static bool traceApi()
    {
        return (s_traceApi < 0 ? readTraceApi() : s_traceApi) != 0;
    }

    /* From now on each indirect call's target into the log once per thread
     * ([CALL]), instead of nothing or (NFS_TRACE_API) every call: what a
     * network race runs, to compare two machines.  Starts the list over. */
    static void traceNewCalls();
    void traceCall(nfs::u32 address);

    /* Reports an indirect call whose target is in no table.  Out of line and
     * cold on purpose: the miss path adds no code to the callers. */
    void reportMissingMethod(nfs::u32 address);

    /* The function registered at `address`, or null.  It is on the hot path --
     * every triangle takes at least two pointer calls, the game's pointer to the
     * driver and the driver's import of Glide -- so it comes out of a table of
     * pages, two loads, rather than the hash map. */
    inline ThunkPtr thunkAt(uint32_t address) const
    {
        const nfs::u32 offset = address - kMethodBase;
        if (offset < kMethodSpan)
        {
            const std::uint16_t* page = m_methodPages[offset >> kMethodPageBits];
            const std::uint16_t index = page ? page[offset & kMethodPageMask] : 0;
            if (index)
                return m_methodList[index].thunk;
        }
        return thunkElsewhere(address);
    }

    /* A call through a pointer (a function pointer, a vtable, an import): the
     * function at `address` run on `args`; its eax.  An address with nothing
     * registered is reported once and skipped, `args` left as they were. */
    inline nfs::u32 callAddress(uint32_t address, nfs::ThreadState& ts, nfs::Args& args)
    {
        const ThunkPtr thunk = thunkAt(address);
        if (!thunk)
        {
            reportMissingMethod(address);
            return args.eax;
        }
        if (traceApi())
            traceCall(address);
        return thunk(this, ts, args);
    }

protected:
    App(const char* appName, nfs::u32 baseAddress, const std::vector<Section>& sections);
    ~App();

    nfs::u32 allocateResourceFixed(GenericResource *resource, nfs::u32 fixedHandle);

private:
    static bool readTraceApi();
    int runOn(nfs::ThreadState& ts, const MemMap& threadStorage, const MemMap& threadStack,
              nfs::u32 entryPoint, nfs::u32 parameter, bool threadLock);
    /* A target outside the page table, or not in it: the map's, or null. */
    ThunkPtr thunkElsewhere(nfs::u32 address) const;

    /* Guest code, and the addresses the port gives the methods of its own
     * libraries, lie between 0x400000 and 0xc00000: the exe from its base and
     * the recompiled DLLs rebased after it. */
    static constexpr nfs::u32 kMethodBase = 0x400000;
    static constexpr nfs::u32 kMethodSpan = 0x800000;
    static constexpr nfs::u32 kMethodPageBits = 12;
    static constexpr nfs::u32 kMethodPageMask = (1u << kMethodPageBits) - 1;

    static int                              s_traceApi;

private:
    nfs::u8*                              m_memory;
    MemMap*                                 m_appName;
    MemMap*                                 m_appNameW;
    MemMap*                                 m_env;
    std::map<nfs::u32, GenericResource*>  m_resources;
    Mutex*                                  m_executionContext;
    Mutex*                                  m_resourceContext;
    /* Safepoints (contextWanted): threads blocked taking the context, since
     * when the first of them waits, and how many times it changed hands after a
     * wait -- which is how a yield knows the waiter got in. */
    std::atomic<unsigned>                   m_contextWaiters{0};
    std::atomic<std::uint64_t>              m_contextWantedSince{0};
    std::atomic<unsigned>                   m_contextHandoffs{0};
    unsigned                                m_yieldPolls = 0;
    void acquireContext(Mutex* executionContext, nfs::u32 depth);

protected:
    std::unordered_map<nfs::u32, Method>  m_methods;
    nfs::ThreadState                                m_ts;

private:
    /* Every registered method once more, for app->callAddress: a page of 4096
     * guest addresses holds, for each, the method's place in m_methodList, 0
     * for none; a page with no method is not allocated.  Filled as methods are
     * registered, which is all done while the application is built. */
    std::vector<Method>                     m_methodList;
    std::uint16_t*                          m_methodPages[kMethodSpan >> kMethodPageBits] = {};
};

class LockContext
{
public:
    LockContext(App& app, const nfs::ThreadState& ts)
        :   m_application(app)
        ,   m_ts(ts)
    {
        m_application.lockContext(ts);
    }
    ~LockContext()
    {
        m_application.unlockContext(m_ts);
    }
private:
    App& m_application;
    const nfs::ThreadState& m_ts;
};

}

#endif
