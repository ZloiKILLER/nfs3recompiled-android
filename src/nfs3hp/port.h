#ifndef NFS3HP_PORT_H
#define NFS3HP_PORT_H

#include "native_guest.h"

#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <optional>

/* The port proper: the game's code rewritten as C++ of its own -- functions
 * with arguments and results, the game's records as structs, its variables
 * by name -- rather than the x86's registers and flat memory.
 *
 * While code that is still the game's (generated or register-level natives)
 * shares the game's memory, a rewritten function keeps its data where the
 * game has it: Var and Table name a variable or a table by its address and
 * read or write it whole (the memory may be unaligned, so through memcpy).
 * It calls what is not rewritten yet through callGame, as Watcom's
 * convention has it.  Called by the game's code, it is entered through an
 * adapter that takes its arguments from the registers and the stack and gives
 * back what Watcom's convention promises the caller: the result in eax, every
 * other register as it was (tools/game_checks holds port:: functions to
 * that, besides memory and the calls they make; the callers' reading of the
 * registers was checked for the sound module: none reads more). */
namespace nfs3hp
{
namespace port
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;
using std::int16_t;
using std::int32_t;
using std::int8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint8_t;

/* A variable of the game's, by its address. */
template <class T>
struct Var
{
    reg32 at;
    T operator()(App* app) const { return app->getMemory<T>(at); }
    void set(App* app, T value) const { app->getMemory<T>(at) = value; }
};

/* A table of the game's records (or values), by its address; `stride` when
 * its entries lie further apart than the record is long. */
template <class T>
struct Table
{
    reg32 at;
    reg32 stride = sizeof(T);
    reg32 address(int32_t i) const { return at + reg32(i) * stride; }
    T operator()(App* app, int32_t i) const
    {
        T value;
        std::memcpy(&value, &app->getMemory<uint8_t>(address(i)), sizeof(T));
        return value;
    }
    void set(App* app, int32_t i, const T& value) const
    {
        std::memcpy(&app->getMemory<uint8_t>(address(i)), &value, sizeof(T));
    }
};

/* A record of the game's where it is: a reference into the game's memory, its
 * fields read and written one at a time, in the order the code has them (T
 * packed, so a field may sit anywhere).  For records that the game's code
 * and the rewrites share, and which the calls between them may change. */
template <class T>
T& record(App* app, reg32 at)
{
    return *&app->getMemory<T>(at);
}

/* A table of such records. */
template <class T>
struct Records
{
    reg32 at;
    reg32 stride = sizeof(T);
    reg32 address(int32_t i) const { return at + reg32(i) * stride; }
    T& operator()(App* app, int32_t i) const { return record<T>(app, address(i)); }
};

/* A record of the game's at an address (on the game's stack, say), read and
 * written whole. */
template <class T>
T load(App* app, reg32 at)
{
    T value;
    std::memcpy(&value, &app->getMemory<uint8_t>(at), sizeof(T));
    return value;
}

template <class T>
void store(App* app, reg32 at, const T& value)
{
    std::memcpy(&app->getMemory<uint8_t>(at), &value, sizeof(T));
}

/* Watcom's register arguments of a call. */
struct Args
{
    reg32 eax = 0, edx = 0, ebx = 0, ecx = 0;
};

/* A call into the game's code (what is not rewritten yet) as Watcom makes
 * it: the register arguments, `stack` pushed, from the game's stack at esp.
 * Its eax; false when the game stops. */
inline bool callGame(App* app, CPU& cpu, reg32 esp, reg32 target, Args a, std::initializer_list<reg32> stack,
                     reg32& result)
{
    cpu.esp = esp;
    cpu.eax = a.eax;
    cpu.edx = a.edx;
    cpu.ebx = a.ebx;
    cpu.ecx = a.ecx;
    if (!guest::call(app, cpu, target, stack))
        return false;
    result = cpu.eax;
    return true;
}

inline bool callGame(App* app, CPU& cpu, reg32 esp, reg32 target, Args a = {}, std::initializer_list<reg32> stack = {})
{
    reg32 ignored;
    return callGame(app, cpu, esp, target, a, stack, ignored);
}

/* The same call of another rewrite's adapter, as the game's code calls its
 * address: the same registers, stack and return address, the adapter run as
 * C++ rather than found through the game's address. */
inline bool callGame(App* app, CPU& cpu, reg32 esp, guest::Native target, Args a, std::initializer_list<reg32> stack,
                     reg32& result)
{
    cpu.esp = esp;
    cpu.eax = a.eax;
    cpu.edx = a.edx;
    cpu.ebx = a.ebx;
    cpu.ecx = a.ecx;
    if (!guest::call(app, cpu, target, stack))
        return false;
    result = cpu.eax;
    return true;
}

inline bool callGame(App* app, CPU& cpu, reg32 esp, guest::Native target, Args a = {},
                     std::initializer_list<reg32> stack = {})
{
    reg32 ignored;
    return callGame(app, cpu, esp, target, a, stack, ignored);
}

/* What a rewritten function that calls the game's code gives back: its
 * result -- what the game's code leaves in eax -- or nothing when the game
 * stopped in a call (cpu.terminate), and then its callers stop as well, up to
 * the adapter, which leaves the x86 as the stop left it. */
using Result = std::optional<int32_t>;

/* Where a rewritten function calls the game's code from: esp as the game's
 * code has it there, so that what is passed on the stack -- and what the
 * called code finds above it -- is where it was.  A rewrite takes the esp
 * its game's code had on entry (at the return address), and its frame is
 * Entry::frame's.
 *
 * Here and there EA's code reads a register no caller set for it -- an
 * uninitialised variable the compiler kept in ebp, say -- and what it does
 * then depends on what the caller had there.  A rewrite reads such a register
 * from cpu, as the game's code finds it: rewritten code leaves esi, edi and
 * ebp alone (the game's code it calls keeps them, Watcom's way), so they hold
 * what the game's caller left -- but where the game's code on the way sets
 * one, the rewrite sets it as well. */

/* ebp as the game's code keeps it while it runs -- its frame pointer -- for
 * what it calls that reads ebp unset; as it was again on the way out. */
struct HeldEbp
{
    CPU& cpu;
    reg32 saved;
    HeldEbp(CPU& c, reg32 ebp) : cpu(c), saved(c.ebp) { c.ebp = ebp; }
    ~HeldEbp() { cpu.ebp = saved; }
    HeldEbp(const HeldEbp&) = delete;
    HeldEbp& operator=(const HeldEbp&) = delete;
};

/* fistp after Watcom's __CHP: `value` towards 0 as an integer, the x87's way
 * (its "integer indefinite" past the integer's range). */
template <class Int>
Int chopped(CPU& cpu, const x86::Float& value)
{
    NFS2_USE(cpu);
    return fp::toInteger<Int>(std::trunc(value));
}

/* The adapter's two ends: the caller's registers and stack on the way in,
 * and on the way out its result in eax, every other register as it was, esp
 * past the return address and `popped` (a stdcall's words). */
struct Entry
{
    reg32 eax, ebx, ecx, edx, esi, edi, ebp, esp;
    explicit Entry(const CPU& c)
        : eax(c.eax), ebx(c.ebx), ecx(c.ecx), edx(c.edx), esi(c.esi), edi(c.edi), ebp(c.ebp), esp(c.esp)
    {
    }
    /* The i-th stack argument (0 the first). */
    reg32 arg(App* app, int i) const { return app->getMemory<reg32>(esp + 4 + 4 * reg32(i)); }
    /* esp as the game's code has it after `pushes` registers and `locals`
     * bytes: where its records on the stack are. */
    reg32 frame(reg32 pushes, reg32 locals = 0) const { return esp - 4 * pushes - locals; }
    void back(CPU& c, reg32 result, reg32 popped = 0) const
    {
        c.eax = result;
        c.ebx = ebx;
        c.ecx = ecx;
        c.edx = edx;
        c.esi = esi;
        c.edi = edi;
        c.ebp = ebp;
        c.esp = esp + 4 + popped;
    }
    /* back() with the result, when the game did not stop. */
    void finish(CPU& c, const Result& result, reg32 popped = 0) const
    {
        if (result)
            back(c, reg32(*result), popped);
    }
};

}  // namespace port
}  // namespace nfs3hp

#endif
