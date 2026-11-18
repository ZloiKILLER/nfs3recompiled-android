# The Watcom C runtime in nfs3.exe

nfs3.exe was linked with the C runtime of Watcom C/C++ 10.x/11.0. From
`0x4df600` on, the runtime's modules lie among EA's libraries -- its memory
manager, files, SND, NET, THRASH's loader, the vector maths -- in the order the
linker took them in, so no address range is the runtime's alone. This is a
map of all of it the game reaches: the 62 functions that the game's own code
and EA's call, everything those call in turn, and the start-up code -- 275
functions. Every other function from `0x4df600` on is EA's.

The names are Open Watcom's (`bld/clib`, `bld/mathlib` of
<https://github.com/open-watcom/open-watcom-v2>), the runtime's own
descendant: each named function was matched against its source there. "(internal)"
is a helper of the runtime not yet put a name to, listed with the family of
what calls it. The calls column counts the direct calls to it in the game's
code; the runtime's functions are also called through pointers, which it does
not count.

The port column says what the port has: `native` for a function of
`src/nfs3hp/native_watcom.cpp` (in `nfs3hp::watcom`, bound to the address by
`disassemble_nfs3hp.py`, `WATCOM_NATIVES`, and checked against the game's code
by `tools/game_checks`), `generated` for the game's own code.  All 275 are
native now.  What the runtime reaches only through pointers the start-up code
sets stays the game's code, outside this map: the x87's formatter
(`0x51f140`, `[__EFG_printf]`) and what `[__EFG_scanf]` points at, the
exception handler and filter `__InitExceptionHandling` installs (`0x51ef08`,
`0x51ed24`), the default `_matherr` (`0x52e3f8`), and the initializers and
finalizers the tables at `0x56b7de` and `0x56b81a` name.

## On the host's C library (plan step 7)

The runtime is native whole; plan step 7 puts what of it the host's C library
does the same on that library, over guest memory, and leaves the rest as the
runtime's code has it -- `tools/game_checks` holds each function to the game's
code either way:

- `strcmp`, `strrchr`, `strstr`: `std::strcmp` (its sign as -1, 0 or 1),
  `std::strrchr`, `std::strstr`.
- `strncpy`, `memcpy`, `memmove`: `strnlen`, `std::memcpy` and `std::memset`
  where the source and the destination share no byte; where they do, the
  runtime's loops, a dword or a byte at a time, whose results the host's
  copies do not promise. `memset` was `std::memset` already.
- `sprintf` and `vsprintf` (and the typed `clib::sprintf` with its words in
  hand): the text made in C++ on the host by `__prtf`'s rules
  (`hostFormatText`) and written into the buffer at once, for the integer
  conversions, `%s`, `%c` and the unknown ones with every flag, width,
  precision and size. `__prtf`'s rules are not C's -- `'0'` pads only an
  integer without a precision, `%#o` of 0 is `00`, `%05s` is not padded -- so
  this is not the host's `snprintf`. A float's conversion (the x87's
  formatter, `%hf`), `%n`, `%p`, the wide ones and far pointers, a buffer that
  overlaps what the text is made from, and the rare register states `__prtf`
  reads through (es over ds not at 0, a 64-bit integer under the direction
  flag) leave the text to `__prtf` as before.

What stays the runtime's own, and why: `_stricmp` (A to Z only, not the host's
locale), `strncmp` (the bytes' difference, not just its sign), `rand` (Watcom's
generator: the race's AI and start draw from it), `qsort` and `bsearch` (the
game's comparator sees the runtime's order of comparisons), the heap (its
blocks' headers and addresses are the game's, and voodoo2a shares it), the
x87's maths and float formatting (to the bit), the streams and the handles
(`FILE`, `__iob` and `__OSHandles` are in guest memory, which other natives
read; under them the port's host functions already do the work), the start,
structured exception handling and the multibyte tables.

## How it is called

Watcom's register convention: arguments in `eax`, `edx`, `ebx` and `ecx`, then
on the stack, which the callee clears; a result in `eax`, a floating-point one
in `st(0)`. A function keeps every register it is not handed an argument in;
those it is handed one in it may leave otherwise, and many do. The variadic
functions (`sprintf`, `fprintf`, `sscanf`) take everything on the stack, which
the caller clears. The runtime's helpers -- `__CHP`, `__STOSB`, `__GRO` and the
`IF@` maths cores -- have conventions of their own, given where they are native.

In native_watcom.cpp (plan step 7) the typed cores take and give back values; the
registers live in the adapters, and in three places besides: the calls through the
runtime's pointers (`hook<Slot>`, `through`, `matherrHook`: its hooks, the comparators,
the scanners' and printers' character routines, the default windowing's hooks), which
load the registers the callee takes and give back its `eax`; the way out's functions
still written register by register (`__fatal_runtime_error`, `__DoneExceptionHandler`,
`exit`, `getcwd`), which the typed `clib::` functions call through `runs`; and the
finally blocks `__local_unwind` calls with `ebp`, `edi` and `ebx`. The pushes a core's
code makes keep their room on the guest's stack and write nothing. The maths run under
their caller's control word, handed down (`Arithmetic`: x86::FPU's rounding at its
precision, fp::'s at 24 bits to nearest); the status word's C3, C2 and C0 come from
`fp::compare`. `__prtf`'s segments and direction flag come down as `Segments` (all 0 in
the game and in the checks' worlds).

## Its data

| Address | What |
|---|---|
| `0x566f58` | `__iob`: 20 `FILE`s of `0x1a` bytes -- `stdin`, `stdout` (`0x566f72`), `stderr` (`0x566f8c`), `stdaux`, `stdprn`; a `FILE` is `_ptr`, `_cnt`, `_link`, `_flag`, `_handle`, `_bufsize`, then the bytes `_ungotten` and `_tmpfchar` |
| `0x9f38a0` | the open streams' list (`__OpenStreams`), `0x9f38a4` the closed ones': `__stream_link`s -- the next, the `FILE`, its buffer, its orientation (none, bytes, wide characters), its extended flags (`_COMMIT` 1) |
| `0x566f4c` | the near heap: `__nheapbeg`, `__MiniHeapRover` (`0x566f50`), `__LargestSizeB4MiniHeapRover` (`0x566f54`); `__MiniHeapFreeRover` at `0x9f3800`, `__nheap_clean` at `0xa0b140`; `__heap_enabled` at `0x56af68`, `_curbrk` at `0x5677f8`, `_amblksiz` at `0x56af6c` |
| `0x567764` | `__GetThreadPtr`: the thread's data -- `errno` at `+4`, `rand`'s seed at `+0xc`, `strtok`'s place at `+0x10`; the single thread's at `[0x9f642c]` |
| `0x567768`.. | the locks: the files' (`0x567768` taken, `0x56776c` given back, the handle in `eax`), the streams' (`0x567778`, `0x56777c`) and the heap's (`__NullAccessRtn` while there is one thread) |
| `0x56acbc` | the handles: `__OSHandles`, Windows' `HANDLE` for each, `__NHandles` of them at `0x56acc0`; `__NFiles` at `0x56ad94` (20); `__io_mode` at `0x56ade8`, a mode for each (`0x56ad98`'s 20); `_fmode` at `0x567164`, `_commode` at `0x568260`, the umask at `0x56ae60` |
| `0x5677b0` | the default windowing's hooks, none in the game: `_WindowsIsWindowedHandle`, `_WindowsStdin` (`0x5677d8`), `_WindowsStdout` (`0x5677dc`) |
| `0x567888` | `__set_errno_dos`'s `errno` for each of Windows' first 20 errors |
| `0x568b94` | the time zone: `timezone`; `daylight` at `0x568b98`, `__dst_adjust` at `0x568b9c`, `tzname[1]` at `0x568b90`; daylight saving's start and end rules at `0x568a40` and `0x568a64`, a `struct tm` each |
| `0x552148` | `__diyr` and `__dilyr` (`0x552162`): the days before each month, shorts |
| `0x564ef0` | `_IsTable`: a byte of flags a character, EOF first (`0x02` space, `0x20` digit) |
| `0x552140` | `_Bits`: 1, 2, 4 .. `0x80`, for the sets `__setbits` makes |
| `0x9f3810` | `atexit`'s 32 functions, their count at `0x9f3890` |
| `0x567858` | `__chipbug`, which `fpatan` is wrapped for |

## The functions

### Strings, characters, numbers

| Address | Name | Open Watcom | Calls to it | Port |
|---|---|---|---|---|
| `0x4e0820` | `strstr` | `clib/string/c/strstr.c` | 17 | native |
| `0x4e0e30` | `strncpy` | `clib/string/c/strncpy.c` | 45 | native |
| `0x4e9ec0` | `_stricmp` | `clib/string/c/stricmp.c` | 17 | native |
| `0x4eb390` | `strncmp` | `clib/string/c/strncmp.c` | 12 | native |
| `0x4ee2f0` | `strrchr` | `clib/string/c/strrchr.c` | 8 | native |
| `0x4ee310` | `strcmp` | `clib/string/a/cmp386.asm` | 54 | native |
| `0x4eede0` | `toupper` | `clib/char/c/toupper.c` | 10 | native |
| `0x4eedf0` | `strncat` | `clib/string/c/strncat.c` | 4 | native |
| `0x4eff60` | `strtok` | `clib/string/c/strtok.c` | 2 | native |
| `0x4f0430` | `atoi` | `clib/convert/c/atoi.c` | 7 | native |
| `0x4f0490` | `atol` | `clib/convert/c/atol.c` | 1 | native |
| `0x4f0520` | `vsscanf` |  | 1 | native |
| `0x4f0548` | `sscanf` | `clib/string/c/sscanf.c` | 8 | native |
| `0x4f0ae0` | `tolower` | `clib/char/c/tolower.c` | 5 | native |
| `0x4f8020` | `_strupr` | `clib/string/c/strupr.c` | 1 | native |
| `0x4f8122` | `__Strtold` |  | 1 | native |
| `0x4f82b6` | `strtod` | `mathlib/c/strtod.c` | 2 | native |
| `0x50c540` | `__setbits` |  | 1 | native |
| `0x50c640` | (internal): `__scnf`'s next character, by the specs' routine |  | 8 | native |
| `0x50c648` | (internal): `__scnf`'s character put back |  | 6 | native |
| `0x50c650` | `__scnf` |  | 1 | native |
| `0x50c904` | (internal): a conversion's options: `*`, width, `N`/`F`, size |  | 1 | native |
| `0x50ca08` | (internal): white space skipped |  | 1 | native |
| `0x50ca48` | (internal): `%c`, `%C` |  | 1 | native |
| `0x50cb30` | (internal): `%s`, `%S` |  | 1 | native |
| `0x50cc88` | (internal): `%n` |  | 1 | native |
| `0x50cce8` | (internal): a `%[` set's bits |  | 1 | native |
| `0x50cd2c` | (internal): `%[` |  | 1 | native |
| `0x50ce18` | (internal): `%e`, `%f`, `%g`: the text to `[__EFG_scanf]` |  | 1 | native |
| `0x50d0f0` | (internal): `%d`, `%i`, `%o`, `%u`, `%x`, `%p` |  | 1 | native |
| `0x50d510` | (internal): a digit's value in any base |  | 1 | native |
| `0x50d538` | (internal): the next character while the width lasts |  | 3 | native |
| `0x5165d8` | `__ZBuf2LD` |  | 1 | native |
| `0x51663a` | (internal): `__ZBuf2LD`'s normalizing |  | 1 | native |
| `0x51667b` | (internal): `__LDScale10x`'s powers of ten |  | 1 | native |
| `0x5166cf` | (internal): the end of `0x51667b`, which `0x5166d8` jumps to |  | 1 | native |
| `0x5166d8` | (internal): `__LDScale10x`'s scaling by one power |  | 1 | native |
| `0x516759` | `__LDScale10x` |  | 3 | native |
| `0x518050` | `_mbslen` |  | 3 | native |
| `0x518080` | `mbstowcs` |  | 3 | native |
| `0x523c00` | `mbtowc` |  | 3 | native |
| `0x523ca0` | `__FDFS` |  | 1 | native |
| `0x523cec` | `__U8M` |  | 1 | native |
| `0x5242f0` | `wcstombs` |  | 1 | native |
| `0x5250c0` | `_mbterm` |  | 4 | native |
| `0x525100` | `_mbsinc` |  | 5 | native |
| `0x52b5b0` | `_mbclen` |  | 5 | native |

### Memory

| Address | Name | Open Watcom | Calls to it | Port |
|---|---|---|---|---|
| `0x4e0640` | `memset` | `clib/memory/c/memset.c` | 84 | native |
| `0x4ff4c0` | `__STOSB` |  | 1 | native |
| `0x4ff4f7` | `__STOSD` |  | 1 | native |

### Random numbers

| Address | Name | Open Watcom | Calls to it | Port |
|---|---|---|---|---|
| `0x4eb1f0` | `initrandnext` |  | 2 | native |
| `0x4eb1fc` | `rand` | `clib/math/c/rand.c` | 6 | native |
| `0x4eb220` | `srand` | `clib/math/c/rand.c` | 1 | native |

### Search and sort

| Address | Name | Open Watcom | Calls to it | Port |
|---|---|---|---|---|
| `0x4ee480` | `med3` | `clib/search/c/qsort.c` | 1 | native |
| `0x4ee4d0` | `byteswap` | `clib/search/c/qsortrtn.c` | 1 | native |
| `0x4ee4f8` | `qsort` | `clib/search/c/qsortrtn.c` | 6 | native |
| `0x4fb300` | `bsearch` | `clib/search/c/bsearch.c` | 3 | native |

### Formatted output

All native (`native_watcom.cpp`): `__prtf` walks the format and hands each
character to its caller's function -- `mem_putc` for `sprintf`, `file_putc`
for `fprintf` -- with the `SPECS` in its own frame; a float's conversion (e,
f, g) goes to `[0x56ac64]` (`__EFG_printf`, the x87's formatter the start-up
code puts there, `0x51f140`), which stays generated.

| Address | Name | Open Watcom | Calls to it | Port |
|---|---|---|---|---|
| `0x4df660` | `mem_putc` |  | 0 | native |
| `0x4df674` | `vsprintf` | `clib/string/c/vsprintf.c` | 11 | native |
| `0x4df690` | `sprintf` | `clib/string/c/sprintf.c` | 198 | native |
| `0x4f4b80` | `fprintf` | `clib/streamio/c/fprintf.c` | 4 | native |
| `0x4fe1d0` | `__prtf` |  | 2 | native |
| `0x4fe578` | (internal): a specification -- flags, width, precision, size |  | 1 | native |
| `0x4fe6cc` | (internal): the flags |  | 1 | native |
| `0x4fe734` | (internal): a string's length, at most the precision |  | 1 | native |
| `0x4fe758` | (internal): a wide string's, in bytes as `wctomb` makes it |  | 1 | native |
| `0x4fe7c4` | (internal): `%p`'s hexadecimal digits, zeros before |  | 1 | native |
| `0x4fe81e` | (internal): the end of `0x4fe7c4`, which `0x4fe828` jumps to |  | 0 | native |
| `0x4fe828` | (internal): `%hf`, a 16.16 fixed-point number |  | 1 | native |
| `0x4fe924` | (internal): `[__EFG_printf]` |  | 1 | native |
| `0x4fe92c` | (internal): `'0'` padding |  | 1 | native |
| `0x4fe970` | (internal): a wide string out by `wctomb` |  | 1 | native |
| `0x4fe9d4` | (internal): a conversion's argument made into text |  | 1 | native |
| `0x4fefdc` | (internal): `strupr` by `toupper`, for `X` and `P` |  | 1 | native |
| `0x4ff000` | `file_putc` |  | 0 | native |
| `0x4ff014` | `vfprintf` | `clib/streamio/c/fprintf.c` | 2 | native |
| `0x51e060` | (internal): `wctomb`'s, by `WideCharToMultiByte` |  | 4 | native |
| `0x51e0a0` | `utoa` | `clib/convert/c/itoa.c` | 1 | native |
| `0x51e0ec` | `itoa` | `clib/convert/c/itoa.c` | 4 | native |
| `0x51e130` | `ulltoa` | `clib/convert/c/lltoa.c` | 2 | native |
| `0x51e220` | (internal): `__prtf`'s `utoa` |  | 2 | native |
| `0x525d59` | `__U8D`: unsigned 64-bit division |  | 2 | native |

### Streams

| Address | Name | Open Watcom | Calls to it | Port |
|---|---|---|---|---|
| `0x4e9030` | `__update_buffer` | `clib/streamio/c/fseek.c` | 1 | native |
| `0x4e9070` | `__reset_buffer` | `clib/streamio/c/fseek.c` | 1 | native |
| `0x4e9088` | `fseek` | `clib/streamio/c/fseek.c` | 11 | native |
| `0x4e9200` | `fread` | `clib/streamio/c/fread.c` | 11 | native |
| `0x4edd80` | `__open_flags` | `clib/streamio/c/fopen.c` | 2 | native |
| `0x4edebc` | `__doopen` | `clib/streamio/c/fopen.c` | 2 | native |
| `0x4edfa4` | `_fsopen` | `clib/streamio/c/fopen.c` | 1 | native |
| `0x4edfe8` | `fopen` | `clib/streamio/c/fopen.c` | 20 | native |
| `0x4ee100` | `fclose` | `clib/streamio/c/fclose.c` | 21 | native |
| `0x4ee144` | `__shutdown_stream` | `clib/streamio/c/fclose.c` | 3 | native |
| `0x4ee15c` | (internal): a hex digit, `__MkTmpFile`'s |  | 1 | native |
| `0x4ee168` | `__MkTmpFile` | `clib/streamio/c/tmputil.c` | 2 | native |
| `0x4ee214` | `__doclose` | `clib/streamio/c/fclose.c` | 2 | native |
| `0x4ee420` | `ftell` | `clib/streamio/c/ftell.c` | 3 | native |
| `0x4eebd0` | `fwrite` | `clib/streamio/c/fwrite.c` | 7 | native |
| `0x4f0570` | `fgets` | `clib/streamio/c/fgets.c` | 2 | native |
| `0x4f7d70` | `fflush` | `clib/streamio/c/fflush.c` | 6 | native |
| `0x4f7d80` | `fputs` | `clib/streamio/c/fputs.c` | 2 | native |
| `0x502760` | `__flush` | `clib/streamio/c/flush.c` | 8 | native |
| `0x5029c0` | `__ioalloc` | `clib/streamio/c/ioalloc.c` | 6 | native |
| `0x502af0` | `fgetc` | `clib/streamio/c/fgetc.c` | 1 | native |
| `0x502bb8` | `__filbuf` | `clib/streamio/c/fgetc.c` | 1 | native |
| `0x502be8` | `__fill_buffer` | `clib/streamio/c/fgetc.c` | 2 | native |
| `0x508ff4` | `_sopen` | `clib/handleio/c/openwnt.c` | 2 | native |
| `0x50901c` | `__sopen` | `clib/handleio/c/openwnt.c` | 1 | native |
| `0x509250` | `__allocfp` | `clib/streamio/c/allocfp.c` | 1 | native |
| `0x509314` | `__freefp` | `clib/streamio/c/allocfp.c` | 2 | native |
| `0x509380` | `__chktty` | `clib/streamio/c/chktty.c` | 2 | native |
| `0x5093c0` | `getpid` | `clib/environ/c/gtpidwnt.c` | 1 | native |
| `0x5094e0` | `__tmpdir` |  | 1 | native |
| `0x5095b0` | `__close` | `clib/handleio/c/_clsewnt.c` | 1 | native |
| `0x5097b0` | `fputc` | `clib/streamio/c/fputc.c` | 4 | native |
| `0x515734` | `__NTConsoleOutput` |  | 2 | native |
| `0x515850` | `flushall` | `clib/streamio/c/flushall.c` | 1 | native |
| `0x515858` | `__flushall` | `clib/streamio/c/flushall.c` | 1 | native |
| `0x51e2b0` | (internal): whether `__OSHandles` has no slot free, `_sopen`'s |  | 1 | native |
| `0x51e4f0` | `__NTGetFakeHandle` | `clib/handleio/c/hdlman.c` | 2 | native |
| `0x522d80` | `fsync` | `clib/handleio/c/fsync.c` | 1 | native |
| `0x522e60` | `getche` | `clib/conio/c/gtchewnt.c` | 1 | native |
| `0x5234d0` | `__GetNTAccessAttr` | `clib/streamio/c/ntfio.c` | 1 | native |
| `0x523504` | `__GetNTShareAttr` | `clib/streamio/c/ntfio.c` | 1 | native |
| `0x523560` | `isatty` | `clib/environ/c/isattwnt.c` | 3 | native |
| `0x523610` | `_fullpath` | `clib/misc/c/fullpath.c` | 1 | native |
| `0x52b290` | (internal): `putch` |  | 1 | native |

A stream's reading, writing and seeking are native, down to the handles'
`__qread`, `__qwrite` and `lseek`: `fputc`, `fputs` and `fwrite` into its
buffer, made by `__ioalloc` (after `__chktty`) the first time, and
`__flush`, `fflush` and `flushall` out of it, `fsync` after where the stream
is committed; `fgetc`, `fgets` and `fread` out of it, filled by
`__fill_buffer` -- by the console's `getche` for an unbuffered tty's handle
0; `fseek` within it where it can (`__update_buffer`)
and `ftell`.  Each keeps esp where its frame has it for what it calls, for
`__qread` and `__qwrite` hand Windows a dword of their own.  So are opening
and closing: `fopen` and `_fsopen` -- `__open_flags`, a stream from
`__allocfp`, `__doopen` -- and `_sopen`'s `CreateFileA` with
`SECURITY_ATTRIBUTES` and the rest of its frame as its code has them, the
handle from `__allocPOSIXHandle`; `fclose`, `__shutdown_stream`,
`__doclose` and `__close`.  `__allocPOSIXHandle` and `__freePOSIXHandle`
are called through `0x567770` and `0x567774`, jumps to them at `0x4ff0f8`
and `0x4ff100`; the handles' lock is at `0x567798` and `0x56779c`.  A
temporary file's name (`__MkTmpFile`, the directory, `getpid`) is still
the game's code, and `_nrealloc`, which grows `__OSHandles`.

### Files and directories

| Address | Name | Open Watcom | Calls to it | Port |
|---|---|---|---|---|
| `0x4e28a0` | `unlink` | `clib/handleio/c/unlnkwnt.c` | 7 | native |
| `0x4eb1d0` | `mkdir` | `clib/direct/c/mkdirwnt.c` | 4 | native |
| `0x4ee2e0` | `remove` | `clib/file/c/remove.c` | 4 | native |
| `0x4ee3c0` | `chmod` | `clib/file/c/chmodwnt.c` | 3 | native |
| `0x4ee400` | `rmdir` | `clib/direct/c/rmdirwnt.c` | 1 | native |
| `0x4f0260` | `chdir` | `clib/direct/c/chdirwnt.c` | 1 | native |
| `0x4f05f0` | `access` | `clib/file/c/accsswnt.c` | 3 | native |
| `0x4f0730` | `getcwd` | `clib/direct/c/gtcwdwnt.c` | 3 | native |
| `0x4f07d0` | `_getdiskfree` | `clib/direct/c/diskfree.c` | 1 | native |
| `0x500f8c` | `__set_errno_dos` | `clib/_dos/c/dosret.c` | 6 | native |
| `0x500ff4` | `__set_errno_nt` | `clib/_dos/c/dosret.c` | 16 | native |
| `0x502880` | `__set_errno` | `clib/startup/c/seterrno.c` | 24 | native |
| `0x5028a4` | `__set_ERANGE` | `clib/startup/c/seterrno.c` | 6 | native |
| `0x5028c8` | `__set_doserrno` | `clib/startup/c/doserrno.c` | 2 | native |
| `0x5028e0` | `lseek` | `clib/handleio/c/lseek.c` | 3 | native |
| `0x502960` | `_tell` | `clib/handleio/c/tell.c` | 2 | native |
| `0x502a50` | `__qread` | `clib/handleio/c/qread.c` | 2 | native |
| `0x509640` | `__fixed_GetFileAttributesA` |  | 1 | native |
| `0x5096c0` | `__qwrite` | `clib/handleio/c/qwrit.c` | 2 | native |
| `0x522de0` | `__GetIOMode` | `clib/handleio/c/iomode.c` | 2 | native |
| `0x522e38` | `__SetIOMode` | `clib/handleio/c/iomode.c` | 4 | native |

### Heap

| Address | Name | Open Watcom | Calls to it | Port |
|---|---|---|---|---|
| `0x4f7900` | `_nmalloc` | `clib/heap/c/nmalloc.c` | 28 | native |
| `0x4f79f0` | `_nfree` | `clib/heap/c/nfree.c` | 32 | native |
| `0x515300` | `__MemAllocator` | `clib/heap/c/mem.c` | 1 | native |
| `0x5153b0` | `__MemFree` | `clib/heap/c/mem.c` | 1 | native |
| `0x5154e0` | `__LinkUpNewNHeap` | `clib/heap/c/grownear.c` | 1 | native |
| `0x515558` | `__CreateNewNHeap` | `clib/heap/c/grownear.c` | 1 | native |
| `0x515614` | `__ExpandDGROUP` | `clib/heap/c/grownear.c` | 1 | native |
| `0x515628` | `__AdjustAmount` | `clib/heap/c/grownear.c` | 1 | native |
| `0x515680` | `__nmemneed` | `clib/heap/c/nmemneed.c` | 1 | native |
| `0x5241c0` | `_nheapshrink` | `clib/heap/c/nheapmin.c` | 2 | native |
| `0x5241fc` | `__ReturnMemToSystem` | `clib/heap/c/nheapmin.c` | 1 | native |
| `0x52425c` | (internal): `__ReturnMemToSystem`, then `__UnlinkNHeap` | `clib/heap/c/nheapmin.c` | 1 | native |
| `0x525578` | (internal): `_nexpand`'s way out |  | 1 | native |

The heap itself is in `src/nfs3hp/native_watcom_heap.cpp`, which voodoo2a's
heap, the same runtime's, shares; `native_watcom.cpp` binds the game's
functions to it. `_nmalloc` and `_nfree` are all the game's code and EA's
call; the rest are called only from these -- `_nheapshrink` also from
`0x51ea7c`, which closes the runtime's locks down at exit.

### x87 and math

| Address | Name | Open Watcom | Calls to it | Port |
|---|---|---|---|---|
| `0x4dfd56` | `__CHP` | `mathlib/a/fchop87.asm` | 148 | native |
| `0x4e06d9` | `IF@DATAN2` | `mathlib/a/atan87.asm` | 4 | native |
| `0x4f1c30` | `floor` | `mathlib/c/floor.c` | 5 | native |
| `0x4f2d78` | `ceil` | `mathlib/c/ceil.c` | 2 | native |
| `0x4f7c90` | `_control87` | `clib/fpu/c/cntrl87.c` | 2 | native |
| `0x4f80b4` | `IF@DEXP` | `mathlib/a/exp87.asm` | 1 | native |
| `0x4f8116` | `exp` | `mathlib/a/exp87.asm` | 1 | native |
| `0x4f9482` | `IF@DACOS` | `mathlib/a/acos87.asm` | 3 | native |
| `0x4ff624` | `__fpatan_wrap` |  | 2 | native |
| `0x502890` | `__set_EDOM` | `clib/startup/c/seterrno.c` | 1 | native |
| `0x50ea4e` | `modf` |  | 1 | native |
| `0x51655c` | `__math87_err` |  | 6 | native |
| `0x51a1ba` | `__@DSQRT` |  | 1 | native |
| `0x51f320` | (internal): `__fpatan_wrap`'s atan2, each division by `__fdiv_fpr` |  | 1 | native |
| `0x5243a7` | `__reporterror` |  | 1 | native |
| `0x5243b1` | (internal): `__reporterror`'s struct exception |  | 2 | native |
| `0x5269d4` | (internal): `__fdiv_fpr`'s safe division |  | 4 | native |
| `0x526aeb` | `__fdiv_fpr` |  | 2 | native |
| `0x52b647` | (internal): the maths' error printed |  | 1 | native |
| `0x52b68f` | (internal): `_matherr`'s call |  | 1 | native |
| `0x52e400` | `__get_std_stream` |  | 1 | native |

The native ones are their instructions as the generated code has them: the
same operations of the port's x87, in the same order -- fldpi and fldl2e as
the doubles it loads -- so that they are its to the bit in either FPU.  So are
the Pentium FDIV bug's (`__fpatan_wrap`, while `__chipbug` says so: an atan2
of a table and a polynomial, each division through `0x526aeb` and
`0x5269d4`; its `fnsave` and `frstor` are not the generated code's either)
and a math error's report (`__reporterror` on: its `struct exception`,
`_matherr` through its pointer, the message to `stderr`, `errno`).

`sscanf` and `strtod` are native too (with the strings, above): `__scnf`
reads through the two routines in its specs (`cget_string` at `0x4f04f0` and
`uncget_string` at `0x4f051c`, sscanf's, native as well), and a float's
conversion gathers the number's text on its stack for `[__EFG_scanf]`;
`strtod`'s long double is built by `__ZBuf2LD` and scaled by
`__LDScale10x` on the x87.

### Time

| Address | Name | Open Watcom | Calls to it | Port |
|---|---|---|---|---|
| `0x4f22b0` | `time` | `clib/time/c/time.c` | 2 | native |
| `0x4f8040` | `_localtime` | `clib/time/c/localtim.c` | 1 | native |
| `0x4f8094` | `localtime` | `clib/time/c/localtim.c` | 1 | native |
| `0x50eb30` | `__getctime` | `clib/time/c/gtctmwnt.c` | 1 | native |
| `0x50eba0` | `mktime` | `clib/time/c/mktime.c` | 2 | native |
| `0x515b34` | (internal): `__tzset`'s without `TZ` |  | 1 | native |
| `0x515c2c` | `__tzset` | `clib/time/c/tzset.c` | 2 | native |
| `0x515c90` | (internal): `__tzset`'s number |  | 2 | native |
| `0x515cbc` | (internal): `__tzset`'s zone |  | 2 | native |
| `0x515dd4` | (internal): `__tzset`'s rule |  | 1 | native |
| `0x515ec2` | (internal): `__tzset`'s daylight saving |  | 1 | native |
| `0x515fa0` | `__brktime` | `clib/time/c/gmtime.c` | 3 | native |
| `0x5160f0` | `__leapyear` | `clib/time/c/leapyear.c` | 4 | native |
| `0x51612c` | `calc_yday` | `clib/time/c/timeutil.c` | 1 | native |
| `0x516224` | `check_order` | `clib/time/c/timeutil.c` | 1 | native |
| `0x516270` | `__isindst` | `clib/time/c/timeutil.c` | 2 | native |
| `0x51652c` | `time_less` | `clib/time/c/timeutil.c` | 1 | native |

`__tzset` and what it calls -- the zone from `TZ` (`0x515cbc`, `0x515c90`,
`0x515dd4`, `0x515ec2`) or from `GetTimeZoneInformation` (`0x515b34`), the
names by `wcstombs` (`0x5242f0`) -- are native too, written a push and a pop
at a time as their code is, `0x515ec2` going on in `__tzset`'s frame.

### Environment, console, process

| Address | Name | Open Watcom | Calls to it | Port |
|---|---|---|---|---|
| `0x4dfd04` | `exit` | `clib/startup/c/exit.c` | 7 | native |
| `0x4dfd30` | `_exit` |  | 1 | native |
| `0x4f0710` | `sleep` | `clib/process/c/sleepwnt.c` | 1 | native |
| `0x4f45c0` | `stackavail` | `clib/startup/c/stackwnt.c` | 2 | native |
| `0x4f4610` | `__GRO` | `clib/startup/a/stkwnt.asm` | 5 | native |
| `0x4f7af8` | `__TryInit` | `clib/startup/a/seh386.asm` | 1 | native |
| `0x4f7b2a` | `__TryHandler` |  | 0 | native |
| `0x4f7b61` | `__TryHandler (scope loop)` |  | 3 | native |
| `0x4f7b7d` | `__Except` | `clib/startup/a/seh386.asm` | 1 | native |
| `0x4f7b8c` | (internal): `__TryHandler`'s end |  | 2 | native |
| `0x4f7ba7` | `__global_unwind` |  | 3 | native |
| `0x4f7bc3` | `__local_unwind` |  | 6 | native |
| `0x4f7c20` | `atexit` | `clib/startup/c/atexit.c` | 3 | native |
| `0x4f7ce0` | `kbhit` | `clib/conio/c/kbhitwnt.c` | 1 | native |
| `0x4f7e20` | `do_getch` |  | 1 | native |
| `0x4f7f24` | `getch` | `clib/conio/c/getchwnt.c` | 2 | native |
| `0x4f7fa0` | `_cstart_` |  | 0 | native |
| `0x4f8b50` | `setenv` | `clib/environ/c/setenv.c` | 2 | native |
| `0x4f8ce8` | `__setenv` |  | 2 | native |
| `0x4f8f38` | (internal): `__findenv` |  | 1 | native |
| `0x4ff0ec` | `__GetThreadPtr` |  | 0 | native |
| `0x4ff0f4` | `__NullAccessRtn` |  | 0 | native |
| `0x4ff108` | `__NTInit` |  | 1 | native |
| `0x4ff338` | (internal): the program's start: `__NTInit`, the initializers |  | 1 | native |
| `0x4ff38c` | `__exit` |  | 2 | native |
| `0x4ff3d0` | (internal): the environment's copies freed |  | 1 | native |
| `0x4ff410` | (internal): `__FiniRtns`' call |  | 2 | native |
| `0x4ff41c` | `__InitRtns` |  | 2 | native |
| `0x4ff46c` | `__FiniRtns` |  | 3 | native |
| `0x50ead0` | `getenv` |  | 7 | native |
| `0x511cd0` | (internal): `__fatal_runtime_error`'s |  | 1 | native |
| `0x511d0c` | `__fatal_runtime_error` |  | 9 | native |
| `0x515690` | `__NTRealKey` |  | 2 | native |
| `0x5156b8` | (internal): the console's handles opened |  | 2 | native |
| `0x515728` | `__NTConsoleInput` |  | 2 | native |
| `0x5158a0` | (internal): the main thread's start |  | 1 | native |
| `0x518290` | `__wsetenv` |  | 2 | native |
| `0x518498` | (internal): `__wfindenv` |  | 1 | native |
| `0x5185c0` | `_nrealloc` |  | 9 | native |
| `0x518640` | `memmove` |  | 8 | native |
| `0x51e304` | `__allocPOSIXHandle` | `clib/handleio/c/hdlman.c` | 3 | native |
| `0x51e45c` | `__freePOSIXHandle` | `clib/handleio/c/hdlman.c` | 0 | native |
| `0x51e490` | `__initPOSIXHandles` | `clib/handleio/c/hdlman.c` | 1 | native |
| `0x51e7ec` | (internal): the main thread's data |  | 2 | native |
| `0x51eb00` | (internal): the module's wide name |  | 1 | native |
| `0x51eba0` | (internal): the command line's copy |  | 1 | native |
| `0x51ebf0` | (internal): the wide command line's copy |  | 1 | native |
| `0x51ec20` | (internal): the stack's limits, by `VirtualQuery` |  | 2 | native |
| `0x51f0d0` | `__InitExceptionHandling` |  | 2 | native |
| `0x51f11c` | `__DoneExceptionHandler` |  | 3 | native |
| `0x523d10` | `_mbsnicmp` |  | 1 | native |
| `0x524190` | `__EnterWVIDEO` |  | 1 | native |
| `0x524290` | (internal): the call of the game's main, then `exit` |  | 1 | native |
| `0x5252a0` | `__create_wide_environment` |  | 4 | native |
| `0x525300` | `wcslen` |  | 7 | native |
| `0x525320` | `memcpy` |  | 4 | native |
| `0x525350` | `wcscpy` |  | 1 | native |
| `0x525370` | (internal): the environment's `towupper` |  | 2 | native |
| `0x525380` | `_msize` |  | 1 | native |
| `0x525390` | `_nexpand` |  | 1 | native |
| `0x5253cc` | `__HeapManager_expand` |  | 1 | native |
| `0x525df0` | (internal): `calloc` |  | 4 | native |
| `0x526030` | (internal): the thread's data's start |  | 1 | native |
| `0x52b5e0` | `_mbcicmp` |  | 1 | native |
| `0x52b640` | (internal): a start-up hook that does nothing |  | 1 | native |
| `0x52c178` | `__wputenv` |  | 3 | native |
| `0x52c300` | (internal): `__wputenv`'s find |  | 1 | native |
| `0x52e310` | `_mbccpy` |  | 2 | native |
| `0x52e350` | `_mbsupr` |  | 1 | native |
| `0x52e3a0` | `_mbccmp` |  | 2 | native |
| `0x52ed30` | `_mbsnextc` |  | 2 | native |
| `0x52ed70` | `_mbctoupper` |  | 1 | native |
| `0x52ede0` | `_mbvtop` |  | 3 | native |
| `0x52eec0` | (internal): `_mbctoupper`'s, code page 932 |  | 1 | native |
| `0x52eee0` | (internal): `_ismbclower`'s, code page 932 |  | 1 | native |
| `0x52ef30` | (internal): `_ismbclegal`'s |  | 1 | native |
| `0x52ef70` | (internal): `_ismbbtrail`'s |  | 1 | native |

