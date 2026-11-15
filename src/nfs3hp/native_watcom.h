#ifndef NFS3HP_NATIVE_WATCOM_H
#define NFS3HP_NATIVE_WATCOM_H

#include "native_guest.h"

/* The Watcom runtime's functions (native_watcom.cpp) as typed C++, for the game's own code that
 * calls them directly (guest::direct); `clib` after the runtime's source tree.  Each works on guest
 * memory alone: what the game's code passed in registers comes as parameters, the result goes back,
 * and every register is kept -- the runtime's code kept all but eax.  When the runtime goes over to
 * the host's C library (plan step 7) only these bodies change.
 *
 * ByAddress: how the game's code calls each, by address -- the arguments in eax, edx, ebx and ecx,
 * the result from eax -- for the checks' stand-ins. */
namespace nfs3hp
{
namespace watcom
{
namespace clib
{

// memset, 0x4e0640: `count` bytes at `buffer` of `value`'s low byte; the buffer.
x86::reg32 memset(win32::WinApplication* app, x86::reg32 buffer, x86::reg32 value, x86::reg32 count);
// strcmp, 0x4ee310: -1, 0 or 1 as the first byte that differs, unsigned, is below or above.
x86::reg32 strcmp(win32::WinApplication* app, x86::reg32 a, x86::reg32 b);
// strncmp, 0x4eb390: at most `count` bytes; the first difference of unsigned bytes, or 0.
x86::reg32 strncmp(win32::WinApplication* app, x86::reg32 a, x86::reg32 b, x86::reg32 count);
// _stricmp, 0x4e9ec0: strcmp's difference with A to Z taken as a to z.
x86::reg32 stricmp(win32::WinApplication* app, x86::reg32 a, x86::reg32 b);
// strncpy, 0x4e0e30: the string, then zeros to make up `count`; the buffer.
x86::reg32 strncpy(win32::WinApplication* app, x86::reg32 buffer, x86::reg32 string, x86::reg32 count);
// strncat, 0x4eedf0: at most `count` bytes of `added` after `string`, always ended; the string.
x86::reg32 strncat(win32::WinApplication* app, x86::reg32 string, x86::reg32 added, x86::reg32 count);
// strrchr, 0x4ee2f0: where `c`'s low byte is last, the end included, or 0.
x86::reg32 strrchr(win32::WinApplication* app, x86::reg32 string, x86::reg32 c);
// strstr, 0x4e0820: where `wanted` first is, or 0 -- the string itself for an empty one.
x86::reg32 strstr(win32::WinApplication* app, x86::reg32 string, x86::reg32 wanted);
// toupper, 0x4eede0, and tolower, 0x4f0ae0: a to z and A to Z moved; anything else as it is.
x86::reg32 toupper(win32::WinApplication* app, x86::reg32 c);
x86::reg32 tolower(win32::WinApplication* app, x86::reg32 c);
// atoi, 0x4f0430 (atol, 0x4f0490): the number, past white space and a sign, wrapping in 32 bits.
x86::reg32 atoi(win32::WinApplication* app, x86::reg32 string);
// __CHP, 0x4dfd56: `value` cut to an integer towards zero (frndint under the chop mode); a NaN or an
// infinity as it is.  The game's code hands it st(0) and takes it back there.
double chop(win32::WinApplication* app, double value);

/* The formatted ones, cdecl: the named arguments and then `words`, the rest, as the game's code
 * pushed them; how many characters (sscanf: how many fields).  They run the runtime's own code as it
 * ran, so they take `cpu`; 0 with cpu.terminate set when the game stopped in one.  Called through
 * guest::direct only: their frame at the depth of the game's call. */
// sprintf, 0x4df690.
x86::reg32 sprintf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buffer, x86::reg32 format,
                   guest::Words words);
// fprintf, 0x4f4b80.
x86::reg32 fprintf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 stream, x86::reg32 format,
                   guest::Words words);
// sscanf, 0x4f0548.
x86::reg32 sscanf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 string, x86::reg32 format,
                  guest::Words words);
// vsprintf, 0x4df674: the arguments' pointer at `arguments` (a va_list).
x86::reg32 vsprintf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buffer, x86::reg32 format,
                    x86::reg32 arguments);

/* The heap's, the streams' and the rest that run more of the runtime: its own code as it ran, the
 * arguments in eax, edx, ebx and ecx, eax back -- 0 with cpu.terminate set when the game stopped. */
// _nmalloc, 0x4f7900: the block, or 0.
x86::reg32 malloc(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 size);
// _nfree, 0x4f79f0.
x86::reg32 free(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 block);
// _nrealloc, 0x5185c0: the block, or 0.
x86::reg32 realloc(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 block, x86::reg32 size);
// calloc, 0x525df0: `count` things of `size` bytes, zeroed; the block, or 0.
x86::reg32 calloc(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 count, x86::reg32 size);
// fopen, 0x4edfe8: the stream, or 0.
x86::reg32 fopen(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name, x86::reg32 mode);
// fclose, 0x4ee100: 0, or -1.
x86::reg32 fclose(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 stream);
// fread, 0x4e9200: how many things read.
x86::reg32 fread(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buffer, x86::reg32 size, x86::reg32 count,
                 x86::reg32 stream);
// fseek, 0x4e9088: 0, or -1.
x86::reg32 fseek(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 stream, x86::reg32 offset,
                 x86::reg32 whence);
// qsort, 0x4ee4f8: `count` things of `size` bytes at `base`, by the game's function at `compare`.
x86::reg32 qsort(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 base, x86::reg32 count, x86::reg32 size,
                 x86::reg32 compare);
// strtok, 0x4eff60: the next token, or 0.
x86::reg32 strtok(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 string, x86::reg32 parts);
// rand, 0x4eb1fc.
x86::reg32 rand(win32::WinApplication* app, x86::CPU& cpu);
// getenv, 0x50ead0: the value, or 0.
x86::reg32 getenv(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name);
// exit, 0x4dfd04: the game ends.
x86::reg32 exit(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 status);
// mkdir, 0x4eb1d0: 0, or -1.
x86::reg32 mkdir(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 path);

}  // namespace clib
}  // namespace watcom

namespace guest
{

/* A runtime function called by address as the game's code calls it: its arguments loaded into
 * eax, edx, ebx and ecx and left there, as the code that loaded them leaves them, its result from
 * eax (0 when the game stopped in it, with cpu.terminate set).  So a check's call goes as the
 * original's did, register for register; the game's direct call keeps them (guest::direct), which
 * the menu check's strict build shows. */
inline void loadWatcomArguments(x86::CPU& cpu, std::initializer_list<x86::reg32> arguments)
{
    x86::reg32* const into[] = {&cpu.eax, &cpu.edx, &cpu.ebx, &cpu.ecx};
    unsigned i = 0;
    for (const x86::reg32 value : arguments)
        *into[i++] = value;
}

template <typename... Args>
struct WatcomByAddress
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, Args... args)
    {
        loadWatcomArguments(cpu, {x86::reg32(args)...});
        return guest::call(app, cpu, Address) ? cpu.eax : 0;
    }
};

using x86::reg32;
template <> struct ByAddress<watcom::clib::memset> : WatcomByAddress<reg32, reg32, reg32> {};
template <> struct ByAddress<watcom::clib::strcmp> : WatcomByAddress<reg32, reg32> {};
template <> struct ByAddress<watcom::clib::strncmp> : WatcomByAddress<reg32, reg32, reg32> {};
template <> struct ByAddress<watcom::clib::stricmp> : WatcomByAddress<reg32, reg32> {};
template <> struct ByAddress<watcom::clib::strncpy> : WatcomByAddress<reg32, reg32, reg32> {};
template <> struct ByAddress<watcom::clib::strncat> : WatcomByAddress<reg32, reg32, reg32> {};
template <> struct ByAddress<watcom::clib::strrchr> : WatcomByAddress<reg32, reg32> {};
template <> struct ByAddress<watcom::clib::strstr> : WatcomByAddress<reg32, reg32> {};
template <> struct ByAddress<watcom::clib::toupper> : WatcomByAddress<reg32> {};
template <> struct ByAddress<watcom::clib::tolower> : WatcomByAddress<reg32> {};
template <> struct ByAddress<watcom::clib::atoi> : WatcomByAddress<reg32> {};
template <> struct ByAddress<watcom::clib::vsprintf> : WatcomByAddress<reg32, reg32, reg32> {};
template <> struct ByAddress<watcom::clib::malloc> : WatcomByAddress<reg32> {};
template <> struct ByAddress<watcom::clib::free> : WatcomByAddress<reg32> {};
template <> struct ByAddress<watcom::clib::realloc> : WatcomByAddress<reg32, reg32> {};
template <> struct ByAddress<watcom::clib::calloc> : WatcomByAddress<reg32, reg32> {};
template <> struct ByAddress<watcom::clib::fopen> : WatcomByAddress<reg32, reg32> {};
template <> struct ByAddress<watcom::clib::fclose> : WatcomByAddress<reg32> {};
template <> struct ByAddress<watcom::clib::fread> : WatcomByAddress<reg32, reg32, reg32, reg32> {};
template <> struct ByAddress<watcom::clib::fseek> : WatcomByAddress<reg32, reg32, reg32> {};
template <> struct ByAddress<watcom::clib::qsort> : WatcomByAddress<reg32, reg32, reg32, reg32> {};
template <> struct ByAddress<watcom::clib::strtok> : WatcomByAddress<reg32, reg32> {};
template <> struct ByAddress<watcom::clib::rand> : WatcomByAddress<> {};
template <> struct ByAddress<watcom::clib::getenv> : WatcomByAddress<reg32> {};
template <> struct ByAddress<watcom::clib::exit> : WatcomByAddress<reg32> {};
template <> struct ByAddress<watcom::clib::mkdir> : WatcomByAddress<reg32> {};

/* A cdecl variadic function called by address: the two named arguments and the words pushed, eax
 * back; the registers as the callee leaves them, as the game's code's call has them. */
struct CdeclByAddress
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 first, x86::reg32 second,
                           Words words)
    {
        return guest::call(app, cpu, Address, {first, second}, words) ? cpu.eax : 0;
    }
};

// __CHP by address: the value pushed on the x87's stack, the function called, its st(0) taken off.
template <>
struct ByAddress<watcom::clib::chop>
{
    template <x86::reg32 Address>
    static double call(win32::WinApplication* app, x86::CPU& cpu, double value)
    {
        cpu.fpu.push(x86::Float(value));
        if (!guest::call(app, cpu, Address))
            return 0.0;
        return double(cpu.fpu.take());
    }
};

template <> struct ByAddress<watcom::clib::sprintf> : CdeclByAddress {};
template <> struct ByAddress<watcom::clib::fprintf> : CdeclByAddress {};
template <> struct ByAddress<watcom::clib::sscanf> : CdeclByAddress {};

}  // namespace guest
}  // namespace nfs3hp

#endif
