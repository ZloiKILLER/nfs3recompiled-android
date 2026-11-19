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
 * While code that is still the game's (register-level natives)
 * shares the game's memory, a rewritten function keeps its data where the
 * game has it: Var and Table name a variable or a table by its address and
 * read or write it whole (the memory may be unaligned, so through memcpy).
 * It calls what is not rewritten yet through callGame, as Watcom's
 * convention has it.  Called by the game's code, it is entered through a
 * thunk that takes its arguments from the registers and the stack and gives
 * back what Watcom's convention promises the caller: the result in eax, every
 * other register as it was. */
namespace nfs3hp
{
namespace port
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;
using guest::heldEbp;
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
    u32 at;
    T operator()(App* app) const { return app->getMemory<T>(at); }
    void set(App* app, T value) const { app->getMemory<T>(at) = value; }
};

/* A table of the game's records (or values), by its address; `stride` when
 * its entries lie further apart than the record is long. */
template <class T>
struct Table
{
    u32 at;
    u32 stride = sizeof(T);
    u32 address(int32_t i) const { return at + u32(i) * stride; }
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
T& record(App* app, u32 at)
{
    return *&app->getMemory<T>(at);
}

/* A table of such records. */
template <class T>
struct Records
{
    u32 at;
    u32 stride = sizeof(T);
    u32 address(int32_t i) const { return at + u32(i) * stride; }
    T& operator()(App* app, int32_t i) const { return record<T>(app, address(i)); }
};

/* A record of the game's at an address (on the game's stack, say), read and
 * written whole. */
template <class T>
T load(App* app, u32 at)
{
    T value;
    std::memcpy(&value, &app->getMemory<uint8_t>(at), sizeof(T));
    return value;
}

template <class T>
void store(App* app, u32 at, const T& value)
{
    std::memcpy(&app->getMemory<uint8_t>(at), &value, sizeof(T));
}

/* Watcom's register arguments of a call. */
struct Args
{
    u32 eax = 0, edx = 0, ebx = 0, ecx = 0;
};

/* A call into the game's code by address as Watcom makes it: the register arguments, `stack`
 * pushed, from the game's stack at esp.  Its eax; false when the game stops. */
inline bool callGame(App* app, ThreadState& ts, u32 esp, u32 target, Args a, std::initializer_list<u32> stack,
                     u32& result)
{
    ts.sp = esp;
    nfs::Args registers;
    registers.ebp = heldEbp();
    registers.eax = a.eax;
    registers.edx = a.edx;
    registers.ebx = a.ebx;
    registers.ecx = a.ecx;
    const std::optional<u32> eax = guest::pointerCall(app, ts, target, registers, stack);
    if (!eax)
        return false;
    result = *eax;
    return true;
}

inline bool callGame(App* app, ThreadState& ts, u32 esp, u32 target, Args a = {}, std::initializer_list<u32> stack = {})
{
    u32 ignored;
    return callGame(app, ts, esp, target, a, stack, ignored);
}

/* What a rewritten function that calls the game's code gives back: its
 * result -- what the game's code leaves in eax -- or nothing when the game
 * stopped in a call (ts.terminate), and then its callers stop as well, up to
 * the thunk, which leaves the x86 as the stop left it. */
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
 * from ts, as the game's code finds it: rewritten code leaves esi, edi and
 * ebp alone (the game's code it calls keeps them, Watcom's way), so they hold
 * what the game's caller left -- but where the game's code on the way sets
 * one, the rewrite sets it as well. */

/* ebp as the game's code keeps it while it runs -- its frame pointer -- for
 * what it calls that reads ebp unset (pointerGame hands it on); as it was
 * again on the way out. */
struct HeldEbp
{
    u32 saved;
    HeldEbp(ThreadState&, u32 ebp) : saved(heldEbp()) { heldEbp() = ebp; }
    ~HeldEbp() { heldEbp() = saved; }
    HeldEbp(const HeldEbp&) = delete;
    HeldEbp& operator=(const HeldEbp&) = delete;
};

/* fistp after Watcom's __CHP: `value` towards 0 as an integer, the x87's way
 * (its "integer indefinite" past the integer's range). */
template <class Int>
Int chopped(ThreadState& ts, const nfs::Float& value)
{
    NFS2_USE(ts);
    return fp::toInteger<Int>(std::trunc(value));
}

/* The thunk's two ends: the caller's registers and stack on the way in,
 * and on the way out its result in eax, every other register as it was, esp
 * past the return address and `popped` (a stdcall's words). */
struct Entry
{
    u32 eax, ebx, ecx, edx, esi, edi, ebp, esp;
    /* The i-th stack argument (0 the first). */
    u32 arg(App* app, int i) const { return app->getMemory<u32>(esp + 4 + 4 * u32(i)); }
    /* esp as the game's code has it after `pushes` registers and `locals`
     * bytes: where its records on the stack are. */
    u32 frame(u32 pushes, u32 locals = 0) const { return esp - 4 * pushes - locals; }
    /* A thunk's (nfs::ThunkPtr) entry: its arguments from `a`, esp the machine's. */
    Entry(const ThreadState& c, const nfs::Args& a)
        : eax(a.eax), ebx(a.ebx), ecx(a.ecx), edx(a.edx), esi(a.esi), edi(a.edi), ebp(a.ebp), esp(c.sp)
    {
    }
    /* A thunk's way back: what back() leaves in the registers left in `a`, esp moved on as the ret
     * moves it; the result returned. */
    u32 back(ThreadState& c, nfs::Args& a, u32 result, u32 popped = 0) const
    {
        a.eax = result;
        a.ebx = ebx;
        a.ecx = ecx;
        a.edx = edx;
        a.esi = esi;
        a.edi = edi;
        a.ebp = ebp;
        c.sp = esp + 4 + popped;
        return result;
    }
    /* back() for a thunk when the game did not stop; 0 when it did (ts.terminate says so). */
    u32 finish(ThreadState& c, nfs::Args& a, const Result& result, u32 popped = 0) const
    {
        return result ? back(c, a, u32(*result), popped) : 0;
    }
};

}  // namespace port
}  // namespace nfs3hp

#endif
