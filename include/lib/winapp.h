#ifndef LIB_WINAPP_H_
#define LIB_WINAPP_H_

#include    <x86.h>
#include    <cpu.h>
#include    <fpu.h>
#include    <lib/memmap.h>
#include    <atomic>
#include    <cstdint>
#include    <map>
#include    <string>
#include    <unordered_map>
#include    <vector>
#include    <SDL3/SDL.h>

namespace win32
{

class WinApplication;
class Mutex;
class MemMap;
class RegistryValue;

/* A function the game calls through a pointer (its tables, callbacks, threads, imports): its
 * arguments in `args` by Watcom's convention, its stack words where cpu.esp points; its eax
 * returned, what else it leaves for the caller in `args`. */
typedef x86::reg32 (*ThunkPtr)(WinApplication* application, x86::CPU& cpu, x86::Args& args);

struct Method
{
    std::string name;
    ThunkPtr thunk = nullptr;

    Method() = default;
    Method(std::string n, ThunkPtr t) : name(std::move(n)), thunk(t) {}
};

class GenericResource
{
    friend class WinApplication;
public:
    virtual ~GenericResource();
    x86::reg32 getResourceIndex() const;

protected:
    GenericResource();

    void setResourceIndex(x86::reg32 resourceIndex);

protected:
    x86::reg32  m_resourceIndex;
};

template< typename T >
class Packed
{
public:
    explicit Packed(x86::reg32 value = 0) : m_ptr(value) {}
    operator x86::reg32() const             { return m_ptr; }

private:
    x86::reg32  m_ptr;
};

template< typename T >
struct MemoryAccessor
{
    friend class WinApplication;
private:
    MemoryAccessor(x86::reg8* address) : address(address) {}
    x86::reg8* address;

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
struct MemoryAccessor<x86::IEEEf80>
{
    friend class WinApplication;
private:
    MemoryAccessor(x86::reg8* address) : address(address) {}
    x86::reg8* address;

public:
    MemoryAccessor(const MemoryAccessor& other) = default;
    MemoryAccessor& operator=(const MemoryAccessor& other) = delete;
    operator x86::Float() const { x86::IEEEf80Data result; memcpy(&result, address, sizeof(x86::IEEEf80Data)); return double(x86::IEEEf80(result)); }
    MemoryAccessor& operator=(x86::Float value) { x86::IEEEf80 f80value(value); memcpy(address, &f80value.data, sizeof(x86::IEEEf80Data)); return *this; }
    operator x86::IEEEf80() const { x86::IEEEf80Data result; memcpy(&result, address, sizeof(x86::IEEEf80Data)); return x86::IEEEf80(result); }
    MemoryAccessor& operator=(x86::IEEEf80 value) { memcpy(address, &value.data, sizeof(x86::IEEEf80Data)); return *this; }
};

template<>
struct MemoryAccessor<void>
{
    friend class WinApplication;
private:
    MemoryAccessor(x86::reg8* address) : address(address) {}
    x86::reg8* address;

public:
    MemoryAccessor(const MemoryAccessor& other) = default;
    MemoryAccessor& operator=(const MemoryAccessor& other) = delete;
    void* operator&() { return reinterpret_cast<void*>(address); }
};

template<>
struct MemoryAccessor<const void>
{
    friend class WinApplication;
private:
    MemoryAccessor(x86::reg8* address) : address(address) {}
    x86::reg8* address;

public:
    MemoryAccessor(const MemoryAccessor& other) = default;
    MemoryAccessor& operator=(const MemoryAccessor& other) = delete;
    const void* operator&() { return reinterpret_cast<const void*>(address); }
};

class WinApplication
{
public:
    GenericResource* getResource(x86::reg32 resource);
    void freeResource(x86::reg32 resource);
    x86::reg32 allocateResource(GenericResource* resource);
    x86::reg32 getAppName() const { return m_appName->getBlockStart(); }
    x86::reg32 getAppNameW() const { return m_appNameW->getBlockStart(); }
    x86::reg32 getEnv() const { return m_env->getBlockStart(); }
    char* getAppNameRaw() { return &getMemory<char>(m_appName->getBlockStart()); }

    void addRegistryKey(x86::reg32 root, const char* keyname, const char* valuename, RegistryValue* value);

    template< typename T >
    inline MemoryAccessor<T> getMemory(x86::reg32 address) { return MemoryAccessor<T>{m_memory + address}; }

    /* Reverse of getMemory(): turn a host pointer that lives inside the guest
     * address space back into the guest address the game itself sees.  Only
     * used by NFS_TRACE_API logging, where knowing *which* guest buffer a
     * string came from is what separates "the game handed us an empty name"
     * from "our layer read the wrong place". */
    inline x86::reg32 guestAddress(const void* pointer) const
    {
        return pointer ? x86::reg32(static_cast<const x86::reg8*>(pointer) - m_memory) : 0;
    }

    void registerMethod(x86::reg32 pointer, Method method);
    /* A method registered already, put another in its place. */
    void replaceMethod(x86::reg32 pointer, Method method);

    int runThread(x86::CPU& cpu, x86::reg32 entryPoint, x86::reg32 parameter = 0, bool threadLock = true);
    /* runThread for callbacks a host thread makes over and over (the timer):
     * the guest stack and thread block stay mapped between calls. */
    int runCallback(x86::CPU& cpu, x86::reg32 entryPoint, x86::reg32 parameter);

    void lockContext(const x86::CPU& cpu);
    void unlockContext(const x86::CPU& cpu);
    void unmarkContext(const x86::CPU& cpu);

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
    void yieldContext(const x86::CPU& cpu);
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
    // Set NFS_TRACE_API=1 in the environment to log every win32 call by name.
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
    void traceCall(x86::reg32 address);

    /* Reports an indirect call whose target is in no table.  Out of line and
     * cold on purpose: the miss path adds no code to the callers. */
    void reportMissingMethod(x86::reg32 address);

    /* The function registered at `address`, or null.  It is on the hot path --
     * every triangle takes at least two pointer calls, the game's pointer to the
     * driver and the driver's import of Glide -- so it comes out of a table of
     * pages, two loads, rather than the hash map. */
    inline ThunkPtr thunkAt(uint32_t address) const
    {
        const x86::reg32 offset = address - kMethodBase;
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
    inline x86::reg32 callAddress(uint32_t address, x86::CPU& cpu, x86::Args& args)
    {
        const ThunkPtr thunk = thunkAt(address);
        if (!thunk)
        {
            reportMissingMethod(address);
            return args.eax;
        }
        if (traceApi())
            traceCall(address);
        return thunk(this, cpu, args);
    }

protected:
    WinApplication(const char* appName, x86::reg32 baseAddress, const std::vector<Section>& sections);
    ~WinApplication();

    x86::reg32 allocateResourceFixed(GenericResource *resource, x86::reg32 fixedHandle);

private:
    static bool readTraceApi();
    int runOn(x86::CPU& cpu, const MemMap& threadStorage, const MemMap& threadStack,
              x86::reg32 entryPoint, x86::reg32 parameter, bool threadLock);
    /* A target outside the page table, or not in it: the map's, or null. */
    ThunkPtr thunkElsewhere(x86::reg32 address) const;

    /* Guest code, and the addresses the port gives the methods of its own
     * libraries, lie between 0x400000 and 0xc00000: the exe from its base and
     * the recompiled DLLs rebased after it. */
    static constexpr x86::reg32 kMethodBase = 0x400000;
    static constexpr x86::reg32 kMethodSpan = 0x800000;
    static constexpr x86::reg32 kMethodPageBits = 12;
    static constexpr x86::reg32 kMethodPageMask = (1u << kMethodPageBits) - 1;

    static int                              s_traceApi;

private:
    x86::reg8*                              m_memory;
    MemMap*                                 m_appName;
    MemMap*                                 m_appNameW;
    MemMap*                                 m_env;
    std::map<x86::reg32, GenericResource*>  m_resources;
    Mutex*                                  m_executionContext;
    Mutex*                                  m_resourceContext;
    /* Safepoints (contextWanted): threads blocked taking the context, since
     * when the first of them waits, and how many times it changed hands after a
     * wait -- which is how a yield knows the waiter got in. */
    std::atomic<unsigned>                   m_contextWaiters{0};
    std::atomic<std::uint64_t>              m_contextWantedSince{0};
    std::atomic<unsigned>                   m_contextHandoffs{0};
    unsigned                                m_yieldPolls = 0;
    void acquireContext(Mutex* executionContext, x86::reg32 depth);

protected:
    std::unordered_map<x86::reg32, Method>  m_methods;
    x86::CPU                                m_cpu;

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
    LockContext(WinApplication& app, const x86::CPU& cpu)
        :   m_application(app)
        ,   m_cpu(cpu)
    {
        m_application.lockContext(cpu);
    }
    ~LockContext()
    {
        m_application.unlockContext(m_cpu);
    }
private:
    WinApplication& m_application;
    const x86::CPU& m_cpu;
};

}

#endif
