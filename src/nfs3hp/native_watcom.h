#ifndef NFS3HP_NATIVE_WATCOM_H
#define NFS3HP_NATIVE_WATCOM_H

#include <cstring>

#include "native_guest.h"

/* The Watcom runtime's functions (native_watcom.cpp) as typed C++, for the game's own code that
 * calls them directly (guest::direct); `clib` after the runtime's source tree.  Each works on guest
 * memory alone: what the game's code passed in registers comes as parameters, the result goes back,
 * and every register is kept -- the runtime's code kept all but eax.  Plan step 7 put these bodies
 * on the host's C library where the result is the runtime's to the byte: strcmp, strrchr, strstr,
 * strncpy, memset (and the runtime's memcpy and memmove) through <cstring> on guest memory, and
 * sprintf's and vsprintf's text made on the host (native_watcom.cpp, hostFormatText); what the
 * host's library would make otherwise -- stricmp's folding, strncmp's difference, rand's numbers,
 * qsort's order, the heap's blocks, the x87's digits -- stays the runtime's own.
 *
 * The game's code calls each by address: the arguments in eax, edx, ebx and ecx, the result from eax. */
namespace nfs3hp
{
namespace watcom
{
namespace clib
{

// memset, 0x4e0640: `count` bytes at `buffer` of `value`'s low byte; the buffer.
nfs::u32 memset(nfs::App* app, nfs::u32 buffer, nfs::u32 value, nfs::u32 count);
// strcmp, 0x4ee310: -1, 0 or 1 as the first byte that differs, unsigned, is below or above.
nfs::u32 strcmp(nfs::App* app, nfs::u32 a, nfs::u32 b);
// strncmp, 0x4eb390: at most `count` bytes; the first difference of unsigned bytes, or 0.
nfs::u32 strncmp(nfs::App* app, nfs::u32 a, nfs::u32 b, nfs::u32 count);
// _stricmp, 0x4e9ec0: strcmp's difference with A to Z taken as a to z.
nfs::u32 stricmp(nfs::App* app, nfs::u32 a, nfs::u32 b);
// strncpy, 0x4e0e30: the string, then zeros to make up `count`; the buffer.
nfs::u32 strncpy(nfs::App* app, nfs::u32 buffer, nfs::u32 string, nfs::u32 count);
// strncat, 0x4eedf0: at most `count` bytes of `added` after `string`, always ended; the string.
nfs::u32 strncat(nfs::App* app, nfs::u32 string, nfs::u32 added, nfs::u32 count);
// strrchr, 0x4ee2f0: where `c`'s low byte is last, the end included, or 0.
nfs::u32 strrchr(nfs::App* app, nfs::u32 string, nfs::u32 c);
// strstr, 0x4e0820: where `wanted` first is, or 0 -- the string itself for an empty one.
nfs::u32 strstr(nfs::App* app, nfs::u32 string, nfs::u32 wanted);
// toupper, 0x4eede0, and tolower, 0x4f0ae0: a to z and A to Z moved; anything else as it is.
nfs::u32 toupper(nfs::App* app, nfs::u32 c);
nfs::u32 tolower(nfs::App* app, nfs::u32 c);
// atoi, 0x4f0430 (atol, 0x4f0490): the number, past white space and a sign, wrapping in 32 bits.
nfs::u32 atoi(nfs::App* app, nfs::u32 string);
// __CHP, 0x4dfd56: `value` cut to an integer towards zero (frndint under the chop mode); a NaN or an
// infinity as it is.  The game's code hands it st(0) and takes it back there.
double chop(nfs::App* app, double value);
// exp, 0x4f8116: e to `power`, which its code leaves on the x87's stack.  0 with ts.terminate set when the game
// stopped.  Called through guest::direct only.
double exponent(nfs::App* app, nfs::ThreadState& ts, double power);
// __reporterror, 0x5243a7: the maths' error `flags` on the double at `argument` reported -- _matherr's
// struct exception, the error printed when it does not take it, errno -- and the result, which its
// code leaves on the x87's stack.  0 with ts.terminate set when the game stopped.  `flags` names
// the error's type, as every caller's does.
double reporterror(nfs::App* app, nfs::ThreadState& ts, nfs::u32 flags, nfs::u32 argument);
// 0x5243b1 (__reporterror's own): the same with the arguments' doubles at `a` and `b`.
double reportError(nfs::App* app, nfs::ThreadState& ts, nfs::u32 flags, nfs::u32 a, nfs::u32 b);

/* The formatted ones, cdecl: the named arguments and then `words`, the rest, as the game's code
 * pushed them; how many characters (sscanf: how many fields).  They run the runtime's own code as it
 * ran, so they take `ts`; 0 with ts.terminate set when the game stopped in one.  Called through
 * guest::direct only: their frame at the depth of the game's call. */
// sprintf, 0x4df690.
nfs::u32 sprintf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 buffer, nfs::u32 format,
                   guest::Words words);
// fprintf, 0x4f4b80.
nfs::u32 fprintf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 stream, nfs::u32 format,
                   guest::Words words);
// sscanf, 0x4f0548.
nfs::u32 sscanf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 string, nfs::u32 format,
                  guest::Words words);
// vsprintf, 0x4df674: the arguments' pointer at `arguments` (a va_list).
nfs::u32 vsprintf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 buffer, nfs::u32 format,
                    nfs::u32 arguments);

/* The heap's, the streams' and the rest that run more of the runtime: its own code as it ran, the
 * arguments in eax, edx, ebx and ecx, eax back -- 0 with ts.terminate set when the game stopped. */
// _nmalloc, 0x4f7900: the block, or 0.
nfs::u32 malloc(nfs::App* app, nfs::ThreadState& ts, nfs::u32 size);
// _nfree, 0x4f79f0.
nfs::u32 free(nfs::App* app, nfs::ThreadState& ts, nfs::u32 block);
// _nrealloc, 0x5185c0: the block, or 0.
nfs::u32 realloc(nfs::App* app, nfs::ThreadState& ts, nfs::u32 block, nfs::u32 size);
// calloc, 0x525df0: `count` things of `size` bytes, zeroed; the block, or 0.
nfs::u32 calloc(nfs::App* app, nfs::ThreadState& ts, nfs::u32 count, nfs::u32 size);
// fopen, 0x4edfe8: the stream, or 0.
nfs::u32 fopen(nfs::App* app, nfs::ThreadState& ts, nfs::u32 name, nfs::u32 mode);
// fclose, 0x4ee100: 0, or -1.
nfs::u32 fclose(nfs::App* app, nfs::ThreadState& ts, nfs::u32 stream);
// fread, 0x4e9200: how many things read.
nfs::u32 fread(nfs::App* app, nfs::ThreadState& ts, nfs::u32 buffer, nfs::u32 size, nfs::u32 count,
                 nfs::u32 stream);
// fseek, 0x4e9088: 0, or -1.
nfs::u32 fseek(nfs::App* app, nfs::ThreadState& ts, nfs::u32 stream, nfs::u32 offset,
                 nfs::u32 whence);
// qsort, 0x4ee4f8: `count` things of `size` bytes at `base`, by the game's function at `compare`.
nfs::u32 qsort(nfs::App* app, nfs::ThreadState& ts, nfs::u32 base, nfs::u32 count, nfs::u32 size,
                 nfs::u32 compare);
// strtok, 0x4eff60: the next token, or 0.
nfs::u32 strtok(nfs::App* app, nfs::ThreadState& ts, nfs::u32 string, nfs::u32 parts);
// rand, 0x4eb1fc.
nfs::u32 rand(nfs::App* app, nfs::ThreadState& ts);
// getenv, 0x50ead0: the value, or 0.
nfs::u32 getenv(nfs::App* app, nfs::ThreadState& ts, nfs::u32 name);
// exit, 0x4dfd04: the game ends.
nfs::u32 exit(nfs::App* app, nfs::ThreadState& ts, nfs::u32 status);
// unlink, 0x4e28a0 (remove, 0x4ee2e0): the file `path` deleted; 0, or -1 and errno.
nfs::u32 unlink(nfs::App* app, nfs::ThreadState& ts, nfs::u32 path);
nfs::u32 remove(nfs::App* app, nfs::ThreadState& ts, nfs::u32 path);
// chdir, 0x4f0260: the current directory `path`; 0, or -1 and errno.
nfs::u32 chdir(nfs::App* app, nfs::ThreadState& ts, nfs::u32 path);
// rmdir, 0x4ee400: the directory `path` removed; 0, or -1 and errno.
nfs::u32 rmdir(nfs::App* app, nfs::ThreadState& ts, nfs::u32 path);
// chmod, 0x4ee3c0: `path` read-only unless `mode` has S_IWRITE (0x80); 0, or -1 and errno.
nfs::u32 chmod(nfs::App* app, nfs::ThreadState& ts, nfs::u32 path, nfs::u32 mode);
// access, 0x4f05f0: 0 if `path` is there, and not read-only when `mode` asks for writing (2); else -1 and errno.
nfs::u32 access(nfs::App* app, nfs::ThreadState& ts, nfs::u32 path, nfs::u32 mode);
// ftell, 0x4ee420: where `stream` is, or -1.
nfs::u32 ftell(nfs::App* app, nfs::ThreadState& ts, nfs::u32 stream);
// vfprintf, 0x4ff014: the format at `format` with the arguments' pointer at `argp` to `stream`; how many characters.
nfs::u32 vfprintf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 stream, nfs::u32 format, nfs::u32 argp);
// fflush, 0x4f7d70: `stream` flushed -- for none, every one -- 0, or -1.
nfs::u32 fflush(nfs::App* app, nfs::ThreadState& ts, nfs::u32 stream);
// mkdir, 0x4eb1d0: 0, or -1.
nfs::u32 mkdir(nfs::App* app, nfs::ThreadState& ts, nfs::u32 path);
// __MkTmpFile, 0x4ee168: a temporary file's name for `number` into `name`; what the code leaves in
// eax, its last digit.
nfs::u32 mkTmpFile(nfs::App* app, nfs::ThreadState& ts, nfs::u32 name, nfs::u32 number);
// srand, 0x4eb220: the generator's seed set to `seed`; the seed's place.
nfs::u32 srand(nfs::App* app, nfs::ThreadState& ts, nfs::u32 seed);
// fwrite, 0x4eebd0: `count` items of `size` bytes from `buffer` to `stream`; how many.
nfs::u32 fwrite(nfs::App* app, nfs::ThreadState& ts, nfs::u32 buffer, nfs::u32 size, nfs::u32 count,
                  nfs::u32 stream);
// time, 0x4f22b0: now as a time_t, also into `timer` unless it is 0.
nfs::u32 time(nfs::App* app, nfs::ThreadState& ts, nfs::u32 timer);
// getcwd, 0x4f0730: the current directory into `buffer`'s `size` bytes (a new block for none); the buffer, or 0.
nfs::u32 getcwd(nfs::App* app, nfs::ThreadState& ts, nfs::u32 buffer, nfs::u32 size);
// _getdiskfree, 0x4f07d0: the drive's free space (1 A:, 0 current) into the diskfree_t at `info`; 0, or -1.
nfs::u32 getdiskfree(nfs::App* app, nfs::ThreadState& ts, nfs::u32 drive, nfs::u32 info);
// stackavail, 0x4f45c0: how far esp is above the thread's stack's bottom (eax is not read).
nfs::u32 stackavail(nfs::App* app, nfs::ThreadState& ts);
// __GRO, 0x4f4610: the stack's pages below esp touched, a dword each, for `size` bytes, handed on the stack (its
// word, below the return address, stays the caller's to push and pop); nothing in eax.
nfs::u32 stackProbed(nfs::App* app, nfs::ThreadState& ts, nfs::u32 size);
// setenv, 0x4f8b50: the variable `name` set to `value`, replacing one there unless `replace` is 0; 0, or -1.
nfs::u32 setenv(nfs::App* app, nfs::ThreadState& ts, nfs::u32 name, nfs::u32 value, nfs::u32 replace);
// atexit, 0x4f7c20: `function` to the 32 run at exit; 0, or -1 when they are full.
nfs::u32 atexit(nfs::App* app, nfs::u32 function);
// getche, 0x522e60: the console's next character, echoed; -1 for none.
nfs::u32 getche(nfs::App* app, nfs::ThreadState& ts);

/* More of the runtime's, for the C runtime's start and end (native_frontend_watc.cpp), with what
 * their code leaves in eax -- 0 with ts.terminate set when the game stopped. */
// __set_errno, 0x502880: errno `value`; the thread's data.
nfs::u32 setErrno(nfs::App* app, nfs::ThreadState& ts, nfs::u32 value);
// __set_ERANGE, 0x5028a4: errno ERANGE; the thread's data.
nfs::u32 setERANGE(nfs::App* app, nfs::ThreadState& ts);
// __set_errno_dos, 0x500f8c: errno for the Windows error `error`; -1.
nfs::u32 setErrnoDos(nfs::App* app, nfs::ThreadState& ts, nfs::u32 error);
// __SetIOMode, 0x522e38: `handle`'s mode `mode`, initialized unless 0; 4 times the handle.
nfs::u32 setIOMode(nfs::App* app, nfs::u32 handle, nfs::u32 mode);
// _mbslen, 0x518050: the characters in the string `s`.
nfs::u32 mbslen(nfs::App* app, nfs::u32 s);
// mbstowcs, 0x518080: the string at `from` into the wide string at `to`, `most` at most; how many, or
// -1.
nfs::u32 mbstowcs(nfs::App* app, nfs::ThreadState& ts, nfs::u32 to, nfs::u32 from, nfs::u32 most);
// __wputenv, 0x52c178: the wide "name=value" `pair` into _wenviron; 0, or -1.  A `pair` with text:
// for an empty one its code reads edx, its by-address callers'.
nfs::u32 wputenv(nfs::App* app, nfs::ThreadState& ts, nfs::u32 pair);
// memmove, 0x518640: `count` bytes from `from` to `to`; `to`.
nfs::u32 memmove(nfs::App* app, nfs::u32 to, nfs::u32 from, nfs::u32 count);
// itoa, 0x51e0ec: `value` in `base` into `out` (signed in base 10); `out`.
nfs::u32 itoa(nfs::App* app, nfs::ThreadState& ts, nfs::u32 value, nfs::u32 out, nfs::u32 base);
// _mbvtop, 0x52ede0: the character `c` into the bytes at `to`; `to`.
nfs::u32 mbvtop(nfs::App* app, nfs::u32 c, nfs::u32 to);
// _mbclen, 0x52b5b0: the character at `p`'s bytes, 1 or 2.
nfs::u32 mbclen(nfs::App* app, nfs::u32 p);
// _mbccmp, 0x52e3a0: the characters at `a` and `b`, their difference.
nfs::u32 mbccmp(nfs::App* app, nfs::u32 a, nfs::u32 b);
// _mbsinc, 0x525100: past the character at `p`.
nfs::u32 mbsinc(nfs::App* app, nfs::u32 p);
// _mbterm, 0x5250c0: whether the string ends at `p`, its character's second byte too.
nfs::u32 mbterm(nfs::App* app, nfs::u32 p);
// __fatal_runtime_error, 0x511d0c: the message at `message` out, and the game ended with `code`.
nfs::u32 fatalRuntimeError(nfs::App* app, nfs::ThreadState& ts, nfs::u32 message, nfs::u32 code);
// _nheapshrink, 0x5241c0: the heap's empty miniheaps given back; 0.
nfs::u32 heapShrink(nfs::App* app, nfs::ThreadState& ts);
// __NTConsoleOutput, 0x515734: the console's output HANDLE.
nfs::u32 consoleOutput(nfs::App* app, nfs::ThreadState& ts);
// strtod, 0x4f82b6: the number at `s` as a double, its end at [end] unless `end` is 0 (what its code
// leaves on the x87's stack); what it leaves in eax into *eax, if asked.
double strtod(nfs::App* app, nfs::ThreadState& ts, nfs::u32 s, nfs::u32 end, nfs::u32* eax);
// fgets, 0x4f0570: the line of `stream` in `buffer`, `n` bytes at most with its 0; the buffer, or 0.
nfs::u32 fgets(nfs::App* app, nfs::ThreadState& ts, nfs::u32 buffer, nfs::u32 n, nfs::u32 stream);
// _strupr, 0x4f8020: the string in capitals, in place; the string.
nfs::u32 strupr(nfs::App* app, nfs::ThreadState& ts, nfs::u32 string);
// __shutdown_stream, 0x4ee144: the stream given up and freed, its handle closed if `close` is not 0; the eax it leaves.
nfs::u32 streamShutdown(nfs::App* app, nfs::ThreadState& ts, nfs::u32 stream, nfs::u32 close);
// __LDScale10x, 0x516759: the long double at `value` times 10 to the `n` (the x87's control word as it is).
void ldScale10x(nfs::App* app, nfs::ThreadState& ts, nfs::u32 value, nfs::u32 n);
// bsearch, 0x4fb300: the element of `count` of `size` bytes at `base` that `function` finds equal to `key`, or 0.
nfs::u32 bsearch(nfs::App* app, nfs::ThreadState& ts, nfs::u32 key, nfs::u32 base, nfs::u32 count,
                   nfs::u32 size, nfs::u32 function);
// __allocPOSIXHandle, 0x51e304: a handle for Windows' HANDLE `os`.
nfs::u32 allocPOSIXHandle(nfs::App* app, nfs::ThreadState& ts, nfs::u32 os);
// __freePOSIXHandle, 0x51e45c: `handle`'s slot free.
nfs::u32 freePOSIXHandle(nfs::App* app, nfs::ThreadState& ts, nfs::u32 handle);
// 0x51e7ec, the main thread's data: the data at `data` -- calloc's, for none -- started; it, or 0.
nfs::u32 initThreadData(nfs::App* app, nfs::ThreadState& ts, nfs::u32 data);
// __InitExceptionHandling, 0x51f0d0: the registration at `registration` the thread's.
nfs::u32 initExceptionHandling(nfs::App* app, nfs::ThreadState& ts, nfs::u32 registration);
// __DoneExceptionHandler, 0x51f11c: the thread's exception record off fs's chain.  `eax` what the
// caller has in eax, which its code does not read.
nfs::u32 doneExceptionHandler(nfs::App* app, nfs::ThreadState& ts, nfs::u32 eax);

}  // namespace clib

// floor (0x4f1c30, stdcall) and ceil (0x4f2d78) of the double `value`, as the game's code leaves it on the x87's stack: the
// value, rounded as the x87's control word has it, the status word left as their compares leave it.  Their frame lies
// where the call with the double pushed puts it (ts.sp is the return address's slot).
nfs::Float floored(nfs::App* app, nfs::ThreadState& ts, nfs::Float value);
nfs::Float ceiled(nfs::App* app, nfs::ThreadState& ts, nfs::Float value);
}  // namespace watcom

namespace guest
{


}  // namespace guest
}  // namespace nfs3hp

#endif
