/* src/nfs3hp/native_game.cpp against the game's generated code, on the
 * desktop, without the game.
 *
 * The game's code for the natives is made afresh from nfs3hp/nfs3.exe by the
 * port's own generator, without the hooks of the tree's copy (generate.py),
 * and built with the port's headers.  Two guest memories hold the same state:
 * the game's data as it loads, what the natives read and write of it made up
 * for every case, buffers, and the stack.  A function runs generated on one
 * and natively on the other, and then they are compared: every call out in
 * order -- the game's own functions, Win32's through its imports, eacsnd's and
 * the renderer's through the pointers the game keeps, the game's callbacks --
 * with what it was handed (its registers and stack words, and what a pointer
 * into the caller's frame points at); the registers the function's caller
 * counts on (kPromises), and eax where it gives something back; the MMX
 * registers; and every byte the functions touch, the stack from the arguments
 * up.  The stack below them
 * is the functions' own and nobody else's.  Every so often all of both
 * memories are compared, for a write anywhere else.
 *
 * What is called is a stand-in, the same for both runs, which keeps to the
 * real one's convention -- Watcom's, which keeps every register but eax and
 * those it is handed an argument in, cdecl or stdcall, which may leave eax,
 * ecx and edx -- and changes what it may, now and then, as
 * the real one might.  The stand-ins' results come from the case's seed and
 * the call's place in the case, the same for both runs as long as both make
 * the same calls; a case may pick a call that stops the game (cpu.terminate),
 * and from there on neither run may do anything.  The functions under test
 * call each other as they do, generated on one side and native on the other.
 *
 * See run.sh. */
#include "nfs3hp.h"
#include <lib/memmap.h>
#include <lib/thread.h>
#include <SDL3/SDL.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <thread>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <random>
#include <string>
#include <vector>
#include <sys/mman.h>

namespace x86
{
void assertLog(const char* file, int line, const char* expr)
{
    std::printf("assert %s:%d %s\n", file, line, expr);
}
}

/* What src/lib/cpu.cpp wants of SDL. */
extern "C"
{
Uint64 SDL_GetPerformanceCounter(void) { return 0; }
Uint64 SDL_GetPerformanceFrequency(void) { return 1; }
const char* SDL_getenv(const char* name) { return std::getenv(name); }
int SDL_strcmp(const char* a, const char* b) { return std::strcmp(a, b); }
}

struct GameSegment
{
    x86::reg32 address, length;
    const x86::reg8* bytes;
};
extern const GameSegment kGameSegments[];
extern const unsigned kGameSegmentCount;

namespace nfs3hp
{
/* native_vertices.cpp's [COMM] trace, which Error calls first: nothing here. */
void commMessageTrace(win32::WinApplication*, x86::CPU&) {}
/* nfs3hp_main.cpp's mark of a race, which takeDownRace clears: nothing here. */
void raceRunning(bool) {}
/* nfs3hp_main.cpp's hooks into the race music (tools/apply_music_stream.py):
 * nothing here, on both sides. */
void musicIndexOpened(win32::WinApplication*, x86::reg32) {}
void musicStreamFile(win32::WinApplication*, x86::reg32) {}
/* nfs3hp_main.cpp's Screen Size list (tools/apply_widescreen.py), on both
 * sides: 16:9 taken, and Dynamic Fullscreen's marker (0xffff) as mode 16. */
bool widescreenMode(x86::reg32 width, x86::reg32 height) { return width * 9 == height * 16; }
x86::reg32 dynamicSavedMode(x86::reg32 found, x86::reg32 width) { return width == 0xffff ? 16 : found; }
/* nfs3hp_main.cpp's mirror drawn in full (tools/apply_mirror_detail.py), on
 * both sides: the mirror's pass (1) answered as the main view's 0. */
x86::reg32 mirrorAsMain(x86::reg32 kind) { return kind == 1 ? 0 : kind; }
}

namespace
{

constexpr x86::reg32 kMemory = 0x1000000;

/* Where things are, in both memories. */
constexpr x86::reg32 kLow = 0, kLowEnd = 0x10000;              // where a null buffer is written
constexpr x86::reg32 kHeap = 0x100000, kHeapEnd = 0x200000;    // VirtualAlloc's: 8 slots
constexpr x86::reg32 kSlotSize = 0x20000;
constexpr x86::reg32 kHarness = 0x600000, kHarnessEnd = 0x640000;
constexpr x86::reg32 kStrings = 0x600000;      // 64 strings of 0x40 bytes
constexpr x86::reg32 kElements = 0x602000;     // qsort's and bsearch's: up to 0xd000 bytes
constexpr x86::reg32 kKey = 0x60f000;          // bsearch's key
constexpr x86::reg32 kAboutRecord = 0x610000;  // what THRASH_about hands back
constexpr x86::reg32 kSamples = 0x620000;      // iSNDconvertsample's source and destination
constexpr x86::reg32 kTexts = 0x634000;        // the string functions': 32 strings of 0x100, in pairs
constexpr x86::reg32 kBuffers = 0x636000;      // what they write: 16 of 0x200
constexpr x86::reg32 kThread = 0x63c000;       // the runtime's per-thread data
constexpr x86::reg32 kSlotsTaken = 0x63e000;   // VirtualAlloc's slots in use, a bit each
constexpr x86::reg32 kHandleTable = 0x63e100;  // the runtime's handles' HANDLEs: 32 of them
constexpr x86::reg32 kGrownTable = 0x63e200;   // where _nrealloc moves them to
constexpr x86::reg32 kStreams = 0x638000;      // 8 of 0x40: a FILE, its __stream_link at +0x20
constexpr x86::reg32 kIobLinks = 0x638200;     // __iob's first three's links, 0x20 each
constexpr x86::reg32 kStreamBuffers = 0x639000;  // the 11's buffers, 0x400 each
constexpr x86::reg32 kStackLow = 0xef0000, kStackHigh = 0xf00000;
/* The MMX functions' (native_mix.cpp), in kSamples: what they read, what they
 * write -- the accumulator, the output -- with room before each for what
 * their loops' ends reach back to; their volumes, positions and decoders'
 * states. */
constexpr x86::reg32 kMmxSource = kSamples + 0x200, kMmxTarget = kSamples + 0x4200;
constexpr x86::reg32 kMmxOutput = kSamples + 0x8200, kMmxState = kSamples + 0xc000;

/* The game's, as native_game.cpp names them. */
constexpr x86::reg32 kWhere = 0x552190, kCalledFrom = 0x55219c, kErrorUnderWay = 0x5521a8;
constexpr x86::reg32 kDebugStop = 0x55e8c0;
constexpr x86::reg32 kErrorPrintOk = 0x564400, kErrorOk = 0x564404, kErrorWhere = 0x564408;
constexpr x86::reg32 kRendererIs3dfx = 0x564410, kErrorFilter = 0x564414, kErrorLock = 0x564418;
constexpr x86::reg32 kErrorBefore = 0x56441c, kDebugOutput = 0x5643fc, kRendererModule = 0x564a6c;
constexpr x86::reg32 kErrorText = 0x56e010;
constexpr x86::reg32 kSoundModule = 0x9ef600, kSoundOptions = 0x9ef608, kSoundFunctions = 0x9ef61c;
constexpr x86::reg32 kWindow = 0x9f3d24, kSoundLock = 0xa09b04, kSoundTick = 0xa09b08;
constexpr x86::reg32 kThrashSlots = 0x9ef900, kThrashSlotsEnd = 0x9ef988;
constexpr x86::reg32 kThreadData = 0x9f642c;  // the Watcom runtime's: its single thread's data
constexpr x86::reg32 kXaNibbles = 0xa0bea0;   // the XA decoders' nibbles, shifted: 16 dwords for each shift

/* The runtime's near heap, as native_watcom_heap.cpp has it (kGameHeap). */
constexpr x86::reg32 kHeapFirst = 0x566f4c, kHeapRover = 0x566f50, kHeapLargestBefore = 0x566f54;
constexpr x86::reg32 kHeapFreeRover = 0x9f3800, kHeapClean = 0xa0b140;
constexpr x86::reg32 kHeapEnabled = 0x56af68, kHeapBrk = 0x5677f8, kHeapBlockSize = 0x56af6c;
constexpr x86::reg32 kHeapAccess = 0x567780, kHeapRelease = 0x567788;
constexpr x86::reg32 kNullAccess = 0x4ff0f4;  // __NullAccessRtn: the lock while there is one thread

/* The runtime's handles, as native_watcom.cpp has them: __OSHandles,
 * __NFiles, __io_mode and the game's 20 modes it points at; the file's lock;
 * the default windowing's hooks. */
constexpr x86::reg32 kOsHandles = 0x56acbc, kFileCount = 0x56ad94, kIoModes = 0x56ade8, kGameIoModes = 0x56ad98;
constexpr x86::reg32 kAccessFile = 0x567768, kReleaseFile = 0x56776c;
constexpr x86::reg32 kIsWindowedHandle = 0x5677b0, kWindowsStdin = 0x5677d8, kWindowsStdout = 0x5677dc;

/* The runtime's streams, as native_watcom.cpp has them: __OpenStreams, their
 * links' list; the streams' lock; __iob, stdin, stdout and stderr first. */
constexpr x86::reg32 kOpenStreams = 0x9f38a0, kAccessIob = 0x567778, kReleaseIob = 0x56777c;
constexpr x86::reg32 kIob = 0x566f58;  // 0x1a bytes each
/* The formatted output's (native_watcom.cpp): the x87's formatter's pointer,
 * the two functions __prtf hands its characters to, and the multibyte
 * tables' switch. */
constexpr x86::reg32 kFloatFormat = 0x56ac64;
constexpr x86::reg32 kMemPutc = 0x4df660, kFilePutc = 0x4ff000;
constexpr x86::reg32 kMbcsOn = 0xa0b200;
constexpr x86::reg32 kClosedStreams = 0x9f38a4;
/* Opening and closing's: __NHandles and __NTGetFakeHandle's last; what
 * calls __allocPOSIXHandle and __freePOSIXHandle, and the handles' lock; the
 * default windowing's other hooks; _commode, _fmode, the umask. */
constexpr x86::reg32 kHandleCount = 0x56acc0, kFakeHandle = 0x56acc4;
constexpr x86::reg32 kAllocHandle = 0x567770, kFreeHandle = 0x567774;
constexpr x86::reg32 kAccessHandles = 0x567798, kReleaseHandles = 0x56779c;
constexpr x86::reg32 kRemoveWindowedHandle = 0x5677b4, kNewWindow = 0x5677b8, kDestroyWindow = 0x5677bc;
constexpr x86::reg32 kCommitMode = 0x568260, kFileMode = 0x567164, kUmask = 0x56ae60;

/* The runtime's time zone, as native_watcom.cpp has it: timezone,
 * __dst_adjust, tzname[1] and daylight saving's rules, a struct tm each. */
constexpr x86::reg32 kTimezone = 0x568b94, kDstAdjust = 0x568b9c, kDstName = 0x568b90;
constexpr x86::reg32 kDstStart = 0x568a40, kDstEnd = 0x568a64;

/* The functions qsort and bsearch are handed: stand-ins (kPointers) by where
 * none of the game's are. */
constexpr x86::reg32 kByDword = 0xfff000, kByByte = 0xfff004, kAtRandom = 0xfff008;
/* And the scopes' filters and finally blocks __TryHandler runs. */
constexpr x86::reg32 kFilterCode = 0xfff010, kFinallyCode = 0xfff014;
/* And the menus': a language's check of a line typed in, the mouse's
 * DirectInput device's GetDeviceData (the device at kMouseDevice, its methods
 * after it). */
constexpr x86::reg32 kLanguageCheck = 0xfff018, kGetDeviceData = 0xfff01c;
constexpr x86::reg32 kMouseDevice = 0x63f000;
/* What the menus' art allocates: 8 blocks of 0x400. */
constexpr x86::reg32 kBlocks = 0x1f0000;

/* What the functions write, and the stack from the arguments up: compared. */
const std::pair<x86::reg32, x86::reg32> kWindows[] = {
    {kLow, kLowEnd}, {kHeap, kHeapEnd}, {kHarness, kHarnessEnd}, {0x552000, 0x553000}, {0x55e000, 0x55f000},
    {0x564000, 0x565000}, {0x566000, 0x568000}, {0x568000, 0x569000}, {0x56a000, 0x56b000}, {0x56b000, 0x56c000},
    {0x56e000, 0x56f000}, {0x563000, 0x564000}, {0x555000, 0x556000}, {0x661000, 0x662000}, {0x6fc000, 0x6fe000},
    {0x554000, 0x555000}, {0x558000, 0x55a000}, {0x561000, 0x562000}, {0x5f3000, 0x5f5000}, {0x664000, 0x665000},
    {0x6fb000, 0x6fc000}, {0x7a3000, 0x7a4000}, {0x7cd000, 0x7ce000}, {0x7f7000, 0x7f8000}, {0x81b000, 0x81c000},
    {0x83f000, 0x841000},
#define CONTROLS_WINDOWS
#include "controls_input.inc"
#include "controls_devices.inc"
#include "controls_race.inc"
#include "controls_pipeline.inc"
#include "controls_queue.inc"
#undef CONTROLS_WINDOWS
#define EA_WINDOWS
#include "ea_threads.inc"
#include "ea_memory.inc"
#include "ea_files.inc"
#include "ea_support.inc"
#include "ea_net.inc"
#include "ea_formats.inc"
#undef EA_WINDOWS
#define LOAD_WINDOWS
#include "load_track.inc"
#include "load_cardb.inc"
#include "load_carmodels.inc"
#include "load_saves.inc"
#undef LOAD_WINDOWS
#define FE_WINDOWS
#include "frontend_glue.inc"
#include "frontend_draw.inc"
#undef FE_WINDOWS
#define DRAW2D_WINDOWS
#include "draw2d.inc"
#undef DRAW2D_WINDOWS
#define TEXT_WINDOWS
#include "text.inc"
#undef TEXT_WINDOWS
#define RACESTART_WINDOWS
#include "racestart.inc"
#undef RACESTART_WINDOWS
#define RACELOAD_WINDOWS
#include "raceload.inc"
#undef RACELOAD_WINDOWS
#define TRACKDATA_WINDOWS
#include "trackdata.inc"
#undef TRACKDATA_WINDOWS
#define PHYSICS_WINDOWS
#include "physics.inc"
#undef PHYSICS_WINDOWS
#define HANDLING_WINDOWS
#include "handling.inc"
#undef HANDLING_WINDOWS
#define DRIVETRAIN_WINDOWS
#include "drivetrain.inc"
#undef DRIVETRAIN_WINDOWS
#define CARSTATE_WINDOWS
#include "carstate.inc"
#undef CARSTATE_WINDOWS
#define TRACK_WINDOWS
#include "track.inc"
#undef TRACK_WINDOWS
#define COLLIDE_WINDOWS
#include "collide.inc"
#undef COLLIDE_WINDOWS
#define AIDRIVE_WINDOWS
#include "aidrive.inc"
#undef AIDRIVE_WINDOWS
#define AI_WINDOWS
#include "ai_race.inc"
#undef AI_WINDOWS
#define PURSUIT_WINDOWS
#include "pursuit.inc"
#undef PURSUIT_WINDOWS
#define AIPLAN_WINDOWS
#include "aiplan.inc"
#undef AIPLAN_WINDOWS
#include "decompiled_windows.inc"
#define GRAPHICS_WINDOWS
#include "graphics_world.inc"
#undef GRAPHICS_WINDOWS
#define SHAPES_WINDOWS
#include "shapes.inc"
#undef SHAPES_WINDOWS
#define FIELDS_WINDOWS
#include "fields.inc"
#undef FIELDS_WINDOWS
#define OBJ3D_WINDOWS
#include "obj3d.inc"
#undef OBJ3D_WINDOWS
#define BUTTONS_WINDOWS
#include "buttons.inc"
#undef BUTTONS_WINDOWS
#define MENUS_WINDOWS
#include "menus.inc"
#undef MENUS_WINDOWS
#define LOOP_WINDOWS
#include "loop.inc"
#undef LOOP_WINDOWS
#define DIALOGS_WINDOWS
#include "dialogs.inc"
#undef DIALOGS_WINDOWS
#define VIEW_WINDOWS
#include "view.inc"
#undef VIEW_WINDOWS
#define HUD_WINDOWS
#include "hud.inc"
#undef HUD_WINDOWS
#define CONV_WINDOWS
#include "conv.inc"
#undef CONV_WINDOWS
#define SOUND_WINDOWS
#include "sound_lib.inc"
#undef SOUND_WINDOWS
    // The fonts' quads, which a text drawn writes its corners into, and their tables.
    {0x7f7000, 0x841000},
    {0x9ef000, 0x9f0000},
    {0x9f3000, 0x9f4000}, {0x9f6000, 0x9f7000}, {0xa09000, 0xa0a000}, {0xa0b000, 0xa0d000}, {kStackLow, kStackHigh},
};

/* The stand-ins' own addresses, for what is called through a pointer. */
constexpr x86::reg32 kStandIns = 0xe00000;  // + 0x10 each of kPointers
constexpr x86::reg32 kNamed = 0xe80000;     // + 0x10 each name GetProcAddress was asked for

enum Convention
{
    kWatcom,   // register arguments; every register kept but those of `clobbers`
    kCdecl,    // arguments on the stack, left there
    kStdcall,  // arguments on the stack, taken off
};

enum Register : unsigned
{
    kEax = 1, kEcx = 2, kEdx = 4, kEbx = 8,
};
constexpr unsigned kScratch = kEax | kEcx | kEdx;

enum Effect
{
    kNothing,
    kText,          // vsprintf: text at eax, its length back
    kPrint,         // sprintf: text at its first argument, its length back
    kLoad,          // LoadLibraryA: a module, or none
    kLookUp,        // GetProcAddress: a stand-in by name, or none
    kVersion,       // iSNDdllversion: 16.3 mostly
    kAbout,         // THRASH_about: kAboutRecord, or none
    kBox,           // MessageBoxA: OK or Cancel
    kMainThread,    // whether this is the main thread
    kAllocate,      // VirtualAlloc: a free slot, filled with samples, or none
    kFree,          // VirtualFree: a slot VirtualAlloc gave, free again
    kDecode,        // a decoder: samples into the buffer
    kFilter,        // the game's say on an error: now and then 0
    kLock,          // the game's lock: a critical section, or none
    kCompare,       // a comparison of qsort's or bsearch's: by kByDword, kByByte or kAtRandom
    kSucceed,       // a Windows function's BOOL: now and then FALSE
    kSomething,     // a result never 0
    kSoundChunk,    // the movies' file streamer's next chunk: one of the sound world's, or none
    kLastError,     // GetLastError: an error the runtime turns into errno one way or another
    kAttributes,    // GetFileAttributesA: a file's attributes, or -1
    kDirectory,     // GetCurrentDirectoryA: a path into the buffer, if it fits, and its length
    kDiskFree,      // GetDiskFreeSpaceA: its four dwords, or FALSE
    kFind,          // FindFirstFileA: WIN32_FIND_DATA, its attributes first, or -1
    kSeek,          // SetFilePointer: a place, or -1
    kRead,          // ReadFile: bytes into the buffer, and how many; or FALSE
    kWrite,         // WriteFile: how many, all of them mostly; or FALSE
    kFileType,      // GetFileType: unknown, disk, character or pipe
    kWindowed,      // _WindowsIsWindowedHandle: a window, or none
    kTransfer,      // _WindowsStdin and _WindowsStdout: how many, -1 now and then
    kLocalTime,     // GetLocalTime: a SYSTEMTIME, sensible mostly
    kZone,          // __tzset: the zone as it was, or another
    kMathResult,    // __reporterror: what _matherr gives back, onto the x87's stack
    kCharacter,     // getche: a byte, or -1
    kOpenFile,      // CreateFileA: a HANDLE, or -1
    kEvent,         // CreateEventA: a HANDLE, or 0
    kHandle,        // __allocPOSIXHandle: a handle -- of the 20 mostly, past them, -1
    kReallocate,    // _nrealloc: the handles' table moved, or 0
    kTmpName,       // __MkTmpFile: a name into the buffer
    kWideChar,      // WideCharToMultiByte: a byte or two into the buffer, or none
    kString,        // the graphics' stand-ins: one of the 64 strings at kStrings
    kRecord,        // the graphics' stand-ins: a record in the graphics' window (graphics_world.inc)
    kBankAdd,       // SNDbankadd: a bank's handle at eax, and 0, 7 (its header to copy) or an error
    kFloat,         // __EFG_printf: a double taken, text into the buffer, the parts' lengths
    kZoneInfo,      // GetTimeZoneInformation: a zone, its names, its biases; which of them is on
    kUpper,         // CharUpperBuffA: the buffer's letters as capitals
    kFullPath,      // GetFullPathNameA: a path into the buffer, if it fits, and its length
    kMultiByte,     // MultiByteToWideChar: a wide character into the buffer, or none
    kScanDouble,    // __EFG_scanf: a double of any bits where its second argument points
    kConsoleInput,  // PeekConsoleInputA, ReadConsoleInputA: a key's record mostly, or none, or FALSE
    kConsoleMode,   // GetConsoleMode: a mode, or FALSE
    kModuleName,    // GetModuleFileNameA and W: a path into the buffer, its length; or 0
    kVersionInfo,   // GetVersion: Windows NT's or 95's, of a version
    kStdHandle,     // GetStdHandle: a HANDLE, or 0, or -1
    kQuery,         // VirtualQuery: the stack's MEMORY_BASIC_INFORMATION about the address
    kCommandLine,   // GetCommandLineA and W: the case's command line, the wide one or none
    kMatherrSay,    // _matherr: 0 mostly -- the error reported -- or 1
    kBlock,         // the game's allocation: one of kBlocks, or none
    kScreenName,    // the shown screen's name: pause, mainmult, postgame, another, or none
    kTickWait,      // sub_4c9860, the wait for the clock: the clock [0x79c1a4] a tick or two on
    kFloatBack,     // a function that leaves a float on the x87's stack: one made up
    kRectangle,     // sub_4beb20: a rectangle's x, y, width, height to [edx], [ebx], [ecx], [[esp+4]]
#define CONTROLS_EFFECTS
#include "controls_input.inc"
#include "controls_devices.inc"
#include "controls_race.inc"
#include "controls_pipeline.inc"
#include "controls_queue.inc"
#undef CONTROLS_EFFECTS
#define EA_EFFECTS
#include "ea_threads.inc"
#include "ea_memory.inc"
#include "ea_files.inc"
#include "ea_support.inc"
#include "ea_net.inc"
#include "ea_formats.inc"
#undef EA_EFFECTS
#define LOAD_EFFECTS
#include "load_track.inc"
#include "load_cardb.inc"
#include "load_carmodels.inc"
#include "load_saves.inc"
#undef LOAD_EFFECTS
#define FE_EFFECTS
#include "frontend_glue.inc"
#include "frontend_draw.inc"
#undef FE_EFFECTS
#define TEXT_EFFECTS
#include "text.inc"
#undef TEXT_EFFECTS
#define BUTTONS_EFFECTS
#include "buttons.inc"
#undef BUTTONS_EFFECTS
#define MENUS_EFFECTS
#include "menus.inc"
#undef MENUS_EFFECTS
#define LOOP_EFFECTS
#include "loop.inc"
#undef LOOP_EFFECTS
#define DIALOGS_EFFECTS
#include "dialogs.inc"
#undef DIALOGS_EFFECTS
#define VIEW_EFFECTS
#include "view.inc"
#undef VIEW_EFFECTS
#define HUD_EFFECTS
#include "hud.inc"
#undef HUD_EFFECTS
#define CONV_EFFECTS
#include "conv.inc"
#undef CONV_EFFECTS
#define RACESTART_EFFECTS
#include "racestart.inc"
#undef RACESTART_EFFECTS
#define RACELOAD_EFFECTS
#include "raceload.inc"
#undef RACELOAD_EFFECTS
#define TRACKDATA_EFFECTS
#include "trackdata.inc"
#undef TRACKDATA_EFFECTS
#define PHYSICS_EFFECTS
#include "physics.inc"
#undef PHYSICS_EFFECTS
#define HANDLING_EFFECTS
#include "handling.inc"
#undef HANDLING_EFFECTS
#define DRIVETRAIN_EFFECTS
#include "drivetrain.inc"
#undef DRIVETRAIN_EFFECTS
#define CARSTATE_EFFECTS
#include "carstate.inc"
#undef CARSTATE_EFFECTS
#define TRACK_EFFECTS
#include "track.inc"
#undef TRACK_EFFECTS
#define COLLIDE_EFFECTS
#include "collide.inc"
#undef COLLIDE_EFFECTS
#define AIDRIVE_EFFECTS
#include "aidrive.inc"
#undef AIDRIVE_EFFECTS
#define AI_EFFECTS
#include "ai_race.inc"
#undef AI_EFFECTS
#define PURSUIT_EFFECTS
#include "pursuit.inc"
#undef PURSUIT_EFFECTS
#define AIPLAN_EFFECTS
#include "aiplan.inc"
#undef AIPLAN_EFFECTS
#define OBJ3D_EFFECTS
#include "obj3d.inc"
#undef OBJ3D_EFFECTS
};

struct StandIn
{
    x86::reg32 address;
    const char* name;
    Convention convention;
    unsigned registers;  // how many of eax, edx, ebx, ecx it takes
    unsigned words;      // stack words it takes
    unsigned clobbers;   // registers it may leave otherwise
    int through;         // where the words of `deref` are read: a register (0-3) or 4 + a stack word, -1 none
    unsigned deref;      // how many
    Effect effect;
    /* It formats: the format in edx and a va_list's address in ebx (1), or
     * the format on the stack and its arguments after it (2).  The arguments
     * the format takes, as many words as it has conversions, are logged too. */
    int formats = 0;
};

/* The game's own functions the code under test calls. */
const StandIn kGameFunctions[] = {
    {0x4df674, "vsprintf", kWatcom, 3, 0, kEax | kEdx | kEbx, 2, 1, kText, 1},
    // sprintf keeps every register but eax, as the runtime's own does (watcom::sprintf's
    // promise): the race's car loading counts on ecx across it.
    {0x4df690, "sprintf", kCdecl, 0, 5, kEax, -1, 0, kPrint},
    {0x4ff014, "vfprintf", kWatcom, 3, 0, kEax | kEdx | kEbx, 2, 1, kNothing, 1},
    // fprintf as graphics::warnTextureMemory calls it: the stream, the format and its three words.
    {0x4f4b80, "fprintf", kCdecl, 0, 5, kScratch, -1, 0, kNothing},
    // port::shapes::shapeToPixels as the graphics' natives call it -- held against its code on its own.
    {0x4d37a0, "shapeToPixels", kWatcom, 4, 1, kEax, -1, 0, kNothing},
    // What the graphics' natives call that hands back a string or a record they read.
    {0x4b6520, "capture folder", kWatcom, 4, 0, kEax, -1, 0, kString},
    {0x439520, "track name", kWatcom, 4, 0, kEax, -1, 0, kRecord},
    {0x422bd0, "car record", kWatcom, 4, 0, kEax, -1, 0, kRecord},
    {0x4f2460, "chunk", kWatcom, 4, 0, kEax, -1, 0, kRecord},
    {0x4ef1e0, "picture", kWatcom, 4, 0, kEax, -1, 0, kRecord},
    {0x4ef190, "new bitmap", kWatcom, 4, 0, kEax, -1, 0, kRecord},
    {0x4df930, "main thread", kWatcom, 1, 0, kEax, -1, 0, kMainThread},
    {0x4dfd04, "exit", kWatcom, 1, 0, kEax, -1, 0, kNothing},
    {0x4ffb64, "stop buffer", kWatcom, 1, 0, kEax, -1, 0, kNothing},
    {0x4ffce0, "buffer position", kWatcom, 3, 0, kEax | kEdx | kEbx, -1, 0, kNothing},
    {0x500864, "mix", kWatcom, 2, 0, kEax | kEdx, -1, 0, kNothing},
    {0x4e08f0, "trace", kCdecl, 0, 2, kScratch, -1, 0, kNothing, 2},
    {0x5008a0, "8 bits", kCdecl, 0, 3, kScratch, -1, 0, kDecode},
    {0x500b10, "16 bits", kCdecl, 0, 3, kScratch, -1, 0, kDecode},
    {0x500990, "XA, two channels", kCdecl, 0, 1, kScratch, 4, 7, kDecode},
    {0x500b90, "XA, one channel", kCdecl, 0, 1, kScratch, 4, 5, kDecode},
    {0x500910, "into the destination", kCdecl, 0, 3, kScratch, -1, 0, kNothing},
    {0x500c80, "the game's tick", kWatcom, 0, 0, kEax, -1, 0, kNothing},
    {0x515c2c, "__tzset", kWatcom, 0, 0, 0, -1, 0, kZone},
    {0x5243a7, "__reporterror", kWatcom, 2, 0, kEax | kEdx, 1, 2, kMathResult},  // the argument, a double
    {0x522e60, "getche", kWatcom, 0, 0, kEax, -1, 0, kCharacter},
    {0x5185c0, "_nrealloc", kWatcom, 2, 0, kEax | kEdx, -1, 0, kReallocate},
    {0x4ee168, "__MkTmpFile", kWatcom, 2, 0, kEax | kEdx, -1, 0, kTmpName},
    {0x4a39f0, "the game's main", kWatcom, 2, 0, kEax | kEdx, -1, 0, kNothing},
    // The cars' names, which the menus' text takes (native_frontend_text.cpp).
    {0x439cc0, "a car known", kWatcom, 1, 0, kEax, -1, 0, kMainThread},
    {0x4396b0, "a police car", kWatcom, 1, 0, kEax, -1, 0, kMainThread},
    {0x439250, "a car's name", kWatcom, 1, 0, kEax, -1, 0, kNothing},
    // What the menus' text and screens call (port_text.cpp,
    // native_frontend_screen.cpp): a text on a 2D screen, a shape by its
    // name, the game's memory, the shown screen's name (the menu loop's,
    // native whole: called by address on both sides).
    {0x4da4a0, "a 2D screen's text", kWatcom, 4, 0, kEax | kEbx | kEcx | kEdx, 0, 4, kNothing},
    {0x4d5db0, "a shape by its name", kWatcom, 1, 0, kEax, 0, 2, kNothing},
    {0x4e1620, "allocate", kWatcom, 3, 0, kEax, -1, 0, kBlock},
    {0x4e1890, "free", kWatcom, 1, 0, kEax, -1, 0, kNothing},
    {0x44ae00, "the shown screen's name", kWatcom, 0, 0, kEax, -1, 0, kScreenName},
#define CONTROLS_GAME_FUNCTIONS
#include "controls_input.inc"
#include "controls_devices.inc"
#include "controls_race.inc"
#include "controls_pipeline.inc"
#include "controls_queue.inc"
#undef CONTROLS_GAME_FUNCTIONS
#define TRACKDATA_GAME_FUNCTIONS
#include "trackdata.inc"
#undef TRACKDATA_GAME_FUNCTIONS
#define PHYSICS_GAME_FUNCTIONS
#include "physics.inc"
#undef PHYSICS_GAME_FUNCTIONS
#define EA_GAME_FUNCTIONS
#include "ea_threads.inc"
#include "ea_memory.inc"
#include "ea_files.inc"
#include "ea_support.inc"
#include "ea_net.inc"
#include "ea_formats.inc"
#undef EA_GAME_FUNCTIONS
#define LOAD_GAME_FUNCTIONS
#include "load_track.inc"
#include "load_cardb.inc"
#include "load_carmodels.inc"
#include "load_saves.inc"
#undef LOAD_GAME_FUNCTIONS
#define FE_GAME_FUNCTIONS
#include "frontend_glue.inc"
#include "frontend_draw.inc"
#undef FE_GAME_FUNCTIONS
#define TEXT_GAME_FUNCTIONS
#include "text.inc"
#undef TEXT_GAME_FUNCTIONS
// Before racestart's: its row for 0x4e06a0 is kept to by every other caller.
#define DRIVETRAIN_GAME_FUNCTIONS
#include "drivetrain.inc"
#undef DRIVETRAIN_GAME_FUNCTIONS
#define RACESTART_GAME_FUNCTIONS
#include "racestart.inc"
#undef RACESTART_GAME_FUNCTIONS
#define RACELOAD_GAME_FUNCTIONS
#include "raceload.inc"
#undef RACELOAD_GAME_FUNCTIONS
#define HANDLING_GAME_FUNCTIONS
#include "handling.inc"
#undef HANDLING_GAME_FUNCTIONS
#define CARSTATE_GAME_FUNCTIONS
#include "carstate.inc"
#undef CARSTATE_GAME_FUNCTIONS
#define TRACK_GAME_FUNCTIONS
#include "track.inc"
#undef TRACK_GAME_FUNCTIONS
#define COLLIDE_GAME_FUNCTIONS
#include "collide.inc"
#undef COLLIDE_GAME_FUNCTIONS
#define AIDRIVE_GAME_FUNCTIONS
#include "aidrive.inc"
#undef AIDRIVE_GAME_FUNCTIONS
#define AI_GAME_FUNCTIONS
#include "ai_race.inc"
#undef AI_GAME_FUNCTIONS
#define PURSUIT_GAME_FUNCTIONS
#include "pursuit.inc"
#undef PURSUIT_GAME_FUNCTIONS
#define AIPLAN_GAME_FUNCTIONS
#include "aiplan.inc"
#undef AIPLAN_GAME_FUNCTIONS
// (No register: the callee reads none, and the menu loop calls it with whatever its own hold.)
{0x4c9860, "the wait for the clock", kWatcom, 0, 0, kEax, -1, 0, kTickWait},
{0x4beb20, "a rectangle copied out", kWatcom, 4, 1, kEdx, -1, 0, kRectangle},
#define SOUND_GAME_FUNCTIONS
#include "sound_lib.inc"
#undef SOUND_GAME_FUNCTIONS
#define SHAPES_GAME_FUNCTIONS
#include "shapes.inc"
#undef SHAPES_GAME_FUNCTIONS
#define FIELDS_GAME_FUNCTIONS
#include "fields.inc"
#undef FIELDS_GAME_FUNCTIONS
#define OBJ3D_GAME_FUNCTIONS
#include "obj3d.inc"
#undef OBJ3D_GAME_FUNCTIONS
#define BUTTONS_GAME_FUNCTIONS
#include "buttons.inc"
#undef BUTTONS_GAME_FUNCTIONS
#define MENUS_GAME_FUNCTIONS
#include "menus.inc"
#undef MENUS_GAME_FUNCTIONS
#define LOOP_GAME_FUNCTIONS
#include "loop.inc"
#undef LOOP_GAME_FUNCTIONS
#define DIALOGS_GAME_FUNCTIONS
#include "dialogs.inc"
#undef DIALOGS_GAME_FUNCTIONS
#define VIEW_GAME_FUNCTIONS
#include "view.inc"
#undef VIEW_GAME_FUNCTIONS
#define HUD_GAME_FUNCTIONS
#include "hud.inc"
#undef HUD_GAME_FUNCTIONS
#define CONV_GAME_FUNCTIONS
#include "conv.inc"
#undef CONV_GAME_FUNCTIONS
// What the decompiled functions call that has no stand-in above (generate.py).
#include "decompiled_standins.inc"
};

/* What is called through a pointer: the game's imports, by their slots, and
 * its callbacks. */
const StandIn kPointers[] = {
    {0x534468, "RegFlushKey", kStdcall, 0, 1, kScratch, -1, 0, kNothing},
    {0x534488, "CloseHandle", kStdcall, 0, 1, kScratch, -1, 0, kSucceed},
    {0x534490, "CreateDirectoryA", kStdcall, 0, 2, kScratch, -1, 0, kSucceed},
    {0x534494, "CreateEventA", kStdcall, 0, 4, kScratch, -1, 0, kEvent},
    {0x534498, "CreateFileA", kStdcall, 0, 7, kScratch, 7, 3, kOpenFile},  // its SECURITY_ATTRIBUTES
    {0x5344a8, "DebugBreak", kStdcall, 0, 0, kScratch, -1, 0, kNothing},
    {0x5344b0, "DeleteFileA", kStdcall, 0, 1, kScratch, -1, 0, kSucceed},
    {0x5344b8, "EnterCriticalSection", kStdcall, 0, 1, kScratch, -1, 0, kNothing},
    {0x5344c0, "ExitProcess", kStdcall, 0, 1, kScratch, -1, 0, kNothing},
    {0x5344cc, "FindFirstFileA", kStdcall, 0, 2, kScratch, -1, 0, kFind},
    {0x5344d4, "FlushFileBuffers", kStdcall, 0, 1, kScratch, -1, 0, kSucceed},
    {0x534500, "GetCurrentDirectoryA", kStdcall, 0, 2, kScratch, -1, 0, kDirectory},
    {0x534514, "GetDiskFreeSpaceA", kStdcall, 0, 5, kScratch, 4, 1, kDiskFree},  // the root's "X:\\"
    {0x534520, "GetFileAttributesA", kStdcall, 0, 1, kScratch, -1, 0, kAttributes},
    {0x53452c, "GetFileType", kStdcall, 0, 1, kScratch, -1, 0, kFileType},
    {0x534534, "GetLastError", kStdcall, 0, 0, kScratch, -1, 0, kLastError},
    {0x534538, "GetLocalTime", kStdcall, 0, 1, kScratch, -1, 0, kLocalTime},
    {0x534558, "GetProcAddress", kStdcall, 0, 2, kScratch, -1, 0, kLookUp},
    {0x534588, "LeaveCriticalSection", kStdcall, 0, 1, kScratch, -1, 0, kNothing},
    {0x53458c, "LoadLibraryA", kStdcall, 0, 1, kScratch, -1, 0, kLoad},
    {0x53459c, "OutputDebugStringA", kStdcall, 0, 1, kScratch, -1, 0, kNothing},
    {0x5345b0, "ReadFile", kStdcall, 0, 5, kScratch, -1, 0, kRead},
    {0x5345b4, "RemoveDirectoryA", kStdcall, 0, 1, kScratch, -1, 0, kSucceed},
    {0x5345d4, "SetCurrentDirectoryA", kStdcall, 0, 1, kScratch, -1, 0, kSucceed},
    {0x5345e4, "SetErrorMode", kStdcall, 0, 1, kScratch, -1, 0, kNothing},
    {0x5345ec, "SetFileAttributesA", kStdcall, 0, 2, kScratch, -1, 0, kSucceed},
    {0x5345f0, "SetFilePointer", kStdcall, 0, 4, kScratch, -1, 0, kSeek},
    {0x534624, "VirtualAlloc", kStdcall, 0, 4, kScratch, -1, 0, kAllocate},
    {0x534628, "VirtualFree", kStdcall, 0, 3, kScratch, -1, 0, kFree},
    {0x534638, "WideCharToMultiByte", kStdcall, 0, 8, kScratch, -1, 0, kWideChar},
    {0x53456c, "GetTimeZoneInformation", kStdcall, 0, 1, kScratch, -1, 0, kZoneInfo},
    {0x5346fc, "CharUpperBuffA", kStdcall, 0, 2, kScratch, -1, 0, kUpper},
    {0x534604, "SleepEx", kStdcall, 0, 2, kScratch, -1, 0, kNothing},
    {0x534508, "GetCurrentProcessId", kStdcall, 0, 0, kScratch, -1, 0, kNothing},
    {0x534530, "GetFullPathNameA", kStdcall, 0, 4, kScratch, -1, 0, kFullPath},
    {0x534598, "MultiByteToWideChar", kStdcall, 0, 6, kScratch, -1, 0, kMultiByte},
    {0x5345dc, "SetEnvironmentVariableA", kStdcall, 0, 2, kScratch, -1, 0, kSucceed},
    {0x534640, "WriteFile", kStdcall, 0, 5, kScratch, -1, 0, kWrite},
    {0x5345a0, "PeekConsoleInputA", kStdcall, 0, 4, kScratch, -1, 0, kConsoleInput},
    {0x5345ac, "ReadConsoleInputA", kStdcall, 0, 4, kScratch, -1, 0, kConsoleInput},
    {0x5344fc, "GetConsoleMode", kStdcall, 0, 2, kScratch, -1, 0, kConsoleMode},
    {0x5345d0, "SetConsoleMode", kStdcall, 0, 2, kScratch, -1, 0, kSucceed},
    {0x53463c, "WriteConsoleA", kStdcall, 0, 5, kScratch, 5, 1, kWrite},  // the character
    {0x5345c0, "RtlUnwind", kStdcall, 0, 4, kScratch, -1, 0, kNothing},
    {0x53454c, "GetModuleHandleA", kStdcall, 0, 1, kScratch, -1, 0, kNothing},
    {0x534544, "GetModuleFileNameA", kStdcall, 0, 3, kScratch, -1, 0, kModuleName},
    {0x534548, "GetModuleFileNameW", kStdcall, 0, 3, kScratch, -1, 0, kModuleName},
    {0x53451c, "GetEnvironmentStrings", kStdcall, 0, 0, kScratch, -1, 0, kNothing},
    {0x534574, "GetVersion", kStdcall, 0, 0, kScratch, -1, 0, kVersionInfo},
    {0x5344f4, "GetCommandLineA", kStdcall, 0, 0, kScratch, -1, 0, kCommandLine},
    {0x5344f8, "GetCommandLineW", kStdcall, 0, 0, kScratch, -1, 0, kCommandLine},
    {0x53455c, "GetStdHandle", kStdcall, 0, 1, kScratch, -1, 0, kStdHandle},
    {0x53462c, "VirtualQuery", kStdcall, 0, 3, kScratch, -1, 0, kQuery},
    {0x534600, "SetUnhandledExceptionFilter", kStdcall, 0, 1, kScratch, -1, 0, kNothing},
    {0x53450c, "GetCurrentThreadId", kStdcall, 0, 0, kScratch, -1, 0, kNothing},
    {0x534764, "MessageBoxA", kStdcall, 0, 4, kScratch, -1, 0, kBox},
    {0x534794, "ShowCursor", kStdcall, 0, 1, kScratch, -1, 0, kNothing},
    {kErrorFilter, "the error filter", kWatcom, 1, 0, kScratch, -1, 0, kFilter},
    {kErrorLock, "the error lock", kWatcom, 0, 0, kScratch, -1, 0, kLock},
    {kErrorBefore, "before the box", kWatcom, 0, 0, kScratch, -1, 0, kNothing},
    {kErrorOk, "OK pressed", kWatcom, 0, 0, kScratch, -1, 0, kNothing},
    {kSoundTick, "a tick of the game's", kWatcom, 0, 0, kScratch, -1, 0, kNothing},
    {kHeapAccess, "the heap's lock taken", kWatcom, 0, 0, 0, -1, 0, kNothing},
    {kHeapRelease, "the heap's lock given back", kWatcom, 0, 0, 0, -1, 0, kNothing},
    {kAccessFile, "the file's lock taken", kWatcom, 1, 0, 0, -1, 0, kNothing},
    {kReleaseFile, "the file's lock given back", kWatcom, 1, 0, 0, -1, 0, kNothing},
    {kAccessIob, "the streams' lock taken", kWatcom, 1, 0, 0, -1, 0, kNothing},
    {kReleaseIob, "the streams' lock given back", kWatcom, 1, 0, 0, -1, 0, kNothing},
    {kAccessHandles, "the handles' lock taken", kWatcom, 1, 0, 0, -1, 0, kNothing},
    {kReleaseHandles, "the handles' lock given back", kWatcom, 1, 0, 0, -1, 0, kNothing},
    {kAllocHandle, "__allocPOSIXHandle", kWatcom, 1, 0, kEax, -1, 0, kHandle},
    {kFreeHandle, "__freePOSIXHandle", kWatcom, 1, 0, kEax, -1, 0, kNothing},
    {kRemoveWindowedHandle, "_WindowsRemoveWindowedHandle", kWatcom, 1, 0, kEax, -1, 0, kNothing},
    {kNewWindow, "_WindowsNewWindow", kCdecl, 4, 3, 0, -1, 0, kNothing},
    {kDestroyWindow, "_WindowsCloseWindow", kWatcom, 1, 0, kEax, -1, 0, kNothing},
    {kIsWindowedHandle, "_WindowsIsWindowedHandle", kWatcom, 1, 0, kEax, -1, 0, kWindowed},
    {kWindowsStdin, "_WindowsStdin", kWatcom, 3, 0, kEax | kEdx | kEbx, -1, 0, kTransfer},
    {kWindowsStdout, "_WindowsStdout", kWatcom, 3, 0, kEax | kEdx | kEbx, -1, 0, kTransfer},
    {kByDword, "compared by dwords", kWatcom, 2, 0, kEax | kEdx, -1, 0, kCompare},
    {kByByte, "compared by bytes", kWatcom, 2, 0, kEax | kEdx, -1, 0, kCompare},
    {kAtRandom, "compared at random", kWatcom, 2, 0, kEax | kEdx, -1, 0, kCompare},
    {kFloatFormat, "__EFG_printf", kWatcom, 3, 0, kEax, -1, 0, kFloat},
    // The way out's hooks and the runtime's finalizers (native_watcom.cpp).
    {0x564484, "an exit hook", kWatcom, 0, 0, kEax, -1, 0, kNothing},
    {0x564488, "another exit hook", kWatcom, 0, 0, kEax, -1, 0, kNothing},
    {0x5677f0, "the program's exit", kWatcom, 0, 0, kEax, -1, 0, kNothing},
    {0x5677a0, "the exit's last", kWatcom, 0, 0, kEax, -1, 0, kNothing},
    {0x5677ac, "an early exit", kWatcom, 2, 0, kEax, -1, 0, kNothing},
    {0x56b800, "a finalizer", kWatcom, 0, 0, kEax, -1, 0, kNothing},
    // A scope's filter, jumped to with its frame in ebp, and a finally block.
    {kFilterCode, "a scope's filter", kWatcom, 0, 3, kEax, -1, 0, kNothing},
    {kFinallyCode, "a finally block", kWatcom, 0, 3, kEax, -1, 0, kNothing},
    // _matherr, handed the struct exception, its 8 words logged.
    {0x56b16c, "_matherr", kWatcom, 1, 0, kEax, 0, 8, kMatherrSay},
    // The console's default windowing's hooks.
    {0x5677e0, "_WindowsKbhit", kWatcom, 1, 0, kEax, -1, 0, kNothing},
    {0x5677e4, "_WindowsGetch", kWatcom, 1, 0, kEax, -1, 0, kCharacter},
    {0x5677ec, "_WindowsPutch", kWatcom, 2, 0, kEax, -1, 0, kNothing},
    // The formatted input's: the float's conversion's number, its first 16 bytes logged.
    {0x56ac68, "__EFG_scanf", kWatcom, 2, 0, kEax | kEdx, 0, 4, kScanDouble},
    // The menus': a language's check of a line, its first 16 bytes logged; the
    // mouse device's GetDeviceData.
    {kLanguageCheck, "a language's check of a line", kWatcom, 1, 0, kEax, 0, 4, kNothing},
    {kGetDeviceData, "GetDeviceData", kStdcall, 0, 5, kScratch, -1, 0, kNothing},
#define CONTROLS_POINTERS
#include "controls_input.inc"
#include "controls_devices.inc"
#include "controls_race.inc"
#include "controls_pipeline.inc"
#include "controls_queue.inc"
#undef CONTROLS_POINTERS
#define EA_POINTERS
#include "ea_threads.inc"
#include "ea_memory.inc"
#include "ea_files.inc"
#include "ea_support.inc"
#include "ea_net.inc"
#include "ea_formats.inc"
#undef EA_POINTERS
#define LOAD_POINTERS
#include "load_track.inc"
#include "load_cardb.inc"
#include "load_carmodels.inc"
#include "load_saves.inc"
#undef LOAD_POINTERS
#define FE_POINTERS
#include "frontend_glue.inc"
#include "frontend_draw.inc"
#undef FE_POINTERS
#define TEXT_POINTERS
#include "text.inc"
#undef TEXT_POINTERS
#define FIELDS_POINTERS
#include "fields.inc"
#undef FIELDS_POINTERS
#define BUTTONS_POINTERS
#include "buttons.inc"
#undef BUTTONS_POINTERS
#define MENUS_POINTERS
#include "menus.inc"
#undef MENUS_POINTERS
#define LOOP_POINTERS
#include "loop.inc"
#undef LOOP_POINTERS
#define DIALOGS_POINTERS
#include "dialogs.inc"
#undef DIALOGS_POINTERS
#define RACESTART_POINTERS
#include "racestart.inc"
#undef RACESTART_POINTERS
#define RACELOAD_POINTERS
#include "raceload.inc"
#undef RACELOAD_POINTERS
#define TRACKDATA_POINTERS
#include "trackdata.inc"
#undef TRACKDATA_POINTERS
#define PHYSICS_POINTERS
#include "physics.inc"
#undef PHYSICS_POINTERS
#define HANDLING_POINTERS
#include "handling.inc"
#undef HANDLING_POINTERS
#define DRIVETRAIN_POINTERS
#include "drivetrain.inc"
#undef DRIVETRAIN_POINTERS
#define CARSTATE_POINTERS
#include "carstate.inc"
#undef CARSTATE_POINTERS
#define TRACK_POINTERS
#include "track.inc"
#undef TRACK_POINTERS
#define COLLIDE_POINTERS
#include "collide.inc"
#undef COLLIDE_POINTERS
#define AIDRIVE_POINTERS
#include "aidrive.inc"
#undef AIDRIVE_POINTERS
#define AI_POINTERS
#include "ai_race.inc"
#undef AI_POINTERS
#define PURSUIT_POINTERS
#include "pursuit.inc"
#undef PURSUIT_POINTERS
#define AIPLAN_POINTERS
#include "aiplan.inc"
#undef AIPLAN_POINTERS
#define SOUND_POINTERS
#include "sound_lib.inc"
#undef SOUND_POINTERS
};
constexpr unsigned kPointerCount = sizeof kPointers / sizeof kPointers[0];

x86::reg32 standInFor(x86::reg32 slot)
{
    for (unsigned i = 0; i < kPointerCount; ++i)
        if (kPointers[i].address == slot)
            return kStandIns + 0x10 * i;
    std::abort();
}

/* The names GetProcAddress was asked for, in the order first asked: stdcall
 * stand-ins, their words from the name's @. */
std::vector<std::string> g_names;

x86::reg32 namedStandIn(const std::string& name)
{
    auto found = std::find(g_names.begin(), g_names.end(), name);
    if (found == g_names.end())
        found = g_names.insert(g_names.end(), name);
    return kNamed + 0x10 * x86::reg32(found - g_names.begin());
}

struct Call
{
    x86::reg32 id;
    std::vector<x86::reg32> words;
    bool operator==(const Call& o) const { return id == o.id && words == o.words; }
};

/* The case: its seed, the call the game stops at, if any, and whether
 * GetProcAddress may not find a name. */
std::uint64_t g_seed = 0;
/* GetCurrentDirectoryA's path within MAX_PATH: getcwd copies as much as it is
 * long from its own MAX_PATH bytes, past them for a longer one -- into what
 * the stack holds there, which natives under the cases that set this (the
 * environment's) leave otherwise than the game's code does. */
bool g_shortDirectory = false;
long g_stopAt = -1;
/* The calls a case's arguments let it make at most (-1 any): for the loops
 * that wait on a thread the stand-ins never are. */
long g_mostCalls = -1;
bool g_lookupFails = false;

std::uint64_t mix(std::uint64_t x)
{
    x += 0x9e3779b97f4a7c15ull;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ull;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebull;
    return x ^ (x >> 31);
}

typedef void (*Function)(win32::WinApplication*, x86::CPU&);

struct Tested
{
    x86::reg32 address;
    const char* name;
    Function generated;
    Function native;
};

const Tested kTested[] = {
#include "game_tested.inc"
};

/* What the runtime's heap is to a function's cases. */
enum HeapUse
{
    kNoHeap,
    kStirred,   // the runtime's heap stirred before each case (stirHeap)
    kRestored,  // and its list as it was after: what it leaves is not what the game would
};

/* What each function promises its caller of the registers, and so what is
 * compared: the game's own callers are Watcom code, which counts on every
 * register it handed no argument in -- of these functions' registers, all
 * but eax; eacsnd calls the lambdas stdcall or cdecl, and counts on ebx, esi,
 * edi and ebp.  eax where there is a result. */
struct Promise
{
    const char* name;
    bool watcom;
    bool result;
    unsigned arguments = 0;  // registers it is handed arguments in -- eax, edx, ebx, ecx -- the caller's no longer
    bool fpu = false;        // the x87's stack and control word compared too
    HeapUse heap = kNoHeap;
    /* Only the x87's registers in use compared, not those past its stack: the
     * generated code keeps the stack in locals and writes the registers only
     * where it calls out, the native pushes and pops each value as the x87
     * does -- what is left in a free register no code reads. */
    bool liveOnly = false;
    /* Registers of ebx, ecx, edx no caller of the game's reads after it (by
     * liveness on the original x86, the values the function passes up
     * followed to the callers' callers), not compared: what a callee the port
     * rewrote (port::, eax and nothing else changed) leaves otherwise there
     * is never seen.  A space-separated list. */
    const char* unread = "";
};

const Promise kPromises[] = {
#include "decompiled_promises.inc"
    {"Error", true, false},
    {"Message", true, true},
    {"AcquireWindow", true, true},
    {"iSNDloaddll", true, true},
    {"SNDcaps", true, true},
    {"Is3DSoundAvailable", true, true},
    {"audiofocushandler", true, false},
    {"SoundServeHandler", true, false},
    {"THRASH_opendll", true, true},
    {"AcquireSoundDataLambda", false, false},
    {"StopSoundBufferLambda", false, false},
    {"AcquireSoundBufferPositionLambda", false, false},
    {"TickSoundLambda", false, false},
    {"PrintSoundLambda", false, false},
    {"TraceSoundLambda", false, false},
    {"abortmessagewrapper", false, false},
    {"iSNDconvertsample", false, true},
    // The game's fill at its three entries: every register, and the x87's
    // stack the doubles go through.
    {"fillZero", true, true, 0, true, kNoHeap, true},
    {"fillByte", true, true, 0, true, kNoHeap, true},
    {"fillWord", true, true, 0, true, kNoHeap, true},
    // The menus' text (native_frontend_text.cpp).
    {"frontend::controllerName", true, true, 1},
    {"frontend::displayModeName", true, true, 1},
    {"frontend::nameValid", true, true, 1},
    // The checks of a line typed in, the screens (native_frontend_text.cpp,
    // native_frontend_screen.cpp): every register compared.
    {"frontend::shortNameCharacter", true, true, 0},
    {"frontend::shortNameValid", true, true, 0},
    {"frontend::languageNameValid", true, true, 0},
    {"frontend::languageFileNameValid", true, true, 0},
    {"frontend::lineAllowed", true, true, 0},
    {"frontend::placePointer", true, true, 0},
    {"frontend::setPointer", true, true, 0},
    {"frontend::loadScreenArt", true, true, 0},
#define CONTROLS_PROMISES
#include "controls_input.inc"
#include "controls_devices.inc"
#include "controls_race.inc"
#include "controls_pipeline.inc"
#include "controls_queue.inc"
#undef CONTROLS_PROMISES
#define EA_PROMISES
#include "ea_threads.inc"
#include "ea_memory.inc"
#include "ea_files.inc"
#include "ea_support.inc"
#include "ea_net.inc"
#include "ea_formats.inc"
#undef EA_PROMISES
#define LOAD_PROMISES
#include "load_track.inc"
#include "load_cardb.inc"
#include "load_carmodels.inc"
#include "load_saves.inc"
#undef LOAD_PROMISES
#define FE_PROMISES
#include "frontend_glue.inc"
#include "frontend_draw.inc"
#undef FE_PROMISES
#define DRAW2D_PROMISES
#include "draw2d.inc"
#undef DRAW2D_PROMISES
#define TEXT_PROMISES
#include "text.inc"
#undef TEXT_PROMISES
#define RACESTART_PROMISES
#include "racestart.inc"
#undef RACESTART_PROMISES
#define RACELOAD_PROMISES
#include "raceload.inc"
#undef RACELOAD_PROMISES
#define TRACKDATA_PROMISES
#include "trackdata.inc"
#undef TRACKDATA_PROMISES
#define PHYSICS_PROMISES
#include "physics.inc"
#undef PHYSICS_PROMISES
#define HANDLING_PROMISES
#include "handling.inc"
#undef HANDLING_PROMISES
#define DRIVETRAIN_PROMISES
#include "drivetrain.inc"
#undef DRIVETRAIN_PROMISES
#define CARSTATE_PROMISES
#include "carstate.inc"
#undef CARSTATE_PROMISES
#define TRACK_PROMISES
#include "track.inc"
#undef TRACK_PROMISES
#define COLLIDE_PROMISES
#include "collide.inc"
#undef COLLIDE_PROMISES
#define AIDRIVE_PROMISES
#include "aidrive.inc"
#undef AIDRIVE_PROMISES
#define AI_PROMISES
#include "ai_race.inc"
#undef AI_PROMISES
#define PURSUIT_PROMISES
#include "pursuit.inc"
#undef PURSUIT_PROMISES
#define AIPLAN_PROMISES
#include "aiplan.inc"
#undef AIPLAN_PROMISES
#define SOUND_PROMISES
#include "sound_lib.inc"
#undef SOUND_PROMISES
#define SHAPES_PROMISES
#include "shapes.inc"
#undef SHAPES_PROMISES
#define FIELDS_PROMISES
#include "fields.inc"
#undef FIELDS_PROMISES
#define OBJ3D_PROMISES
#include "obj3d.inc"
#undef OBJ3D_PROMISES
#define BUTTONS_PROMISES
#include "buttons.inc"
#undef BUTTONS_PROMISES
#define MENUS_PROMISES
#include "menus.inc"
#undef MENUS_PROMISES
#define LOOP_PROMISES
#include "loop.inc"
#undef LOOP_PROMISES
#define DIALOGS_PROMISES
#include "dialogs.inc"
#undef DIALOGS_PROMISES
#define VIEW_PROMISES
#include "view.inc"
#undef VIEW_PROMISES
#define HUD_PROMISES
#include "hud.inc"
#undef HUD_PROMISES
#define CONV_PROMISES
#include "conv.inc"
#undef CONV_PROMISES
    // The Watcom runtime's (native_watcom.cpp).
    {"watcom::CHP", true, false, 0, true},
    {"watcom::GetThreadPtr", true, true},
    {"watcom::NullAccess", true, false},
    {"watcom::memset", true, true, 3},
    {"watcom::STOSB", true, true},
    {"watcom::STOSD", true, true},
    {"watcom::strcmp", true, true, 2},
    {"watcom::strncmp", true, true, 3},
    {"watcom::stricmp", true, true, 2},
    {"watcom::strncpy", true, true, 3},
    {"watcom::strncat", true, true, 3},
    {"watcom::strrchr", true, true, 2},
    {"watcom::strstr", true, true, 2},
    {"watcom::setbits", true, false, 2},
    {"watcom::strtok", true, true, 2},
    {"watcom::strupr", true, true, 1},
    {"watcom::toupper", true, true, 1},
    {"watcom::tolower", true, true, 1},
    {"watcom::atoi", true, true, 1},
    {"watcom::atol", true, true, 1},
    {"watcom::initrandnext", true, true},
    {"watcom::rand", true, true},
    {"watcom::srand", true, false, 1},
    {"watcom::set_errno", true, true, 1},
    {"watcom::set_ERANGE", true, true},
    {"watcom::set_doserrno", true, true, 1},
    {"watcom::set_errno_dos", true, true, 1},
    {"watcom::set_errno_nt", true, true},
    {"watcom::unlink", true, true, 1},
    {"watcom::remove", true, true, 1},
    {"watcom::mkdir", true, true, 1},
    {"watcom::rmdir", true, true, 1},
    {"watcom::chdir", true, true, 1},
    {"watcom::chmod", true, true, 2},
    {"watcom::access", true, true, 2},
    {"watcom::getcwd", true, true, 2},
    {"watcom::getdiskfree", true, true, 2},
    {"watcom::fixed_GetFileAttributesA", true, true, 1},
    {"watcom::GetIOMode", true, true, 1},
    {"watcom::SetIOMode", true, true, 2},
    {"watcom::isatty", true, true, 1},
    {"watcom::lseek", true, true, 3},
    {"watcom::tell", true, true, 1},
    {"watcom::qread", true, true, 3},
    {"watcom::qwrite", true, true, 3},
    {"watcom::leapyear", true, true, 1},
    {"watcom::brktime", true, true, 4},
    {"watcom::time_less", true, true, 2},
    {"watcom::calc_yday", true, true, 2},
    {"watcom::check_order", true, true, 3},
    {"watcom::isindst", true, true, 1},
    {"watcom::mktime", true, true, 1},
    {"watcom::getctime", true, true, 1},
    {"watcom::time", true, true, 1},
    {"watcom::localtime_", true, true, 2},
    {"watcom::localtime", true, true, 1},
    {"watcom::IF_DATAN2", true, true, 0, true},
    {"watcom::floor", true, true, 0, true},
    {"watcom::ceil", true, true, 0, true},
    {"watcom::modf", true, true, 0, true},
    {"watcom::control87", true, true, 2, true},
    {"watcom::IF_DEXP", true, true, 0, true},
    {"watcom::exp", true, true, 0, true},
    {"watcom::IF_DACOS", true, true, 0, true},
    {"watcom::DSQRT", true, true, 0, true},
    {"watcom::math87_err", true, true, 0, true},
    {"watcom::set_EDOM", true, true},
    {"watcom::med3", true, true, 4},
    {"watcom::byteswap", true, true},  // esi, edi and ecx its arguments, all it leaves as qsort expects
    {"watcom::qsort", true, false, 4},
    {"watcom::bsearch", true, true, 4},
    {"watcom::fsync", true, true, 1},
    {"watcom::chktty", true, true, 1},
    {"watcom::ioalloc", true, true, 1},
    {"watcom::flush", true, true, 1},
    {"watcom::fputc", true, true, 2},
    {"watcom::fputs", true, true, 2},
    {"watcom::fwrite", true, true, 4},
    {"watcom::flushall_", true, true, 1},
    {"watcom::flushall", true, true},
    {"watcom::fflush", true, true, 1},
    {"watcom::update_buffer", true, true, 2},
    {"watcom::reset_buffer", true, true, 1},
    {"watcom::fill_buffer", true, true, 1},
    {"watcom::filbuf", true, true, 1},
    {"watcom::fgetc", true, true, 1},
    {"watcom::fread", true, true, 4},
    {"watcom::fgets", true, true, 3},
    {"watcom::fseek", true, true, 3},
    {"watcom::ftell", true, true, 1},
    {"watcom::GetNTAccessAttr", true, true, 3},
    {"watcom::GetNTShareAttr", true, true, 2},
    {"watcom::handlesFull", true, true},
    {"watcom::NTGetFakeHandle", true, true},
    {"watcom::allocPOSIXHandle", true, true, 1},
    {"watcom::freePOSIXHandle", true, true, 1},
    {"watcom::close", true, true, 1},
    {"watcom::sopen_", true, true, 4},
    {"watcom::sopen", true, true},
    {"watcom::open_flags", true, true, 2},
    {"watcom::doopen", true, true, 4},
    {"watcom::allocfp", true, true, 1},
    {"watcom::freefp", true, true, 1},
    {"watcom::doclose", true, true, 2},
    {"watcom::shutdown_stream", true, true, 2},
    {"watcom::fclose", true, true, 1},
    {"watcom::fsopen", true, true, 3},
    {"watcom::fopen", true, true, 2},
    // Its formatted output: sprintf and fprintf, cdecl, keep every register
    // but eax; __U8D's results are all four, compared as kept ones.
    {"watcom::mem_putc", true, true, 2},
    {"watcom::file_putc", true, true, 2},
    {"watcom::vsprintf", true, true, 3},
    {"watcom::sprintf", true, true},
    {"watcom::vfprintf", true, true, 3},
    {"watcom::fprintf", true, true},
    {"watcom::prtf", true, true, 4},
    {"watcom::prtfSpec", true, true, 3},
    {"watcom::prtfFlags", true, true, 2},
    {"watcom::prtfLength", true, true, 3},
    {"watcom::prtfWideLength", true, true, 3},
    {"watcom::prtfHex", true, true, 3},
    {"watcom::prtfHexEnd", true, true},
    {"watcom::prtfFixed", true, false, 3},
    {"watcom::prtfFloat", true, false, 3},
    {"watcom::prtfZeros", true, true, 1},
    {"watcom::prtfWideOut", true, false, 4},
    {"watcom::prtfConvert", true, true, 4},
    {"watcom::prtfUpper", true, true, 1},
    {"watcom::wctomb_", true, true, 2},
    {"watcom::utoa", true, true, 3},
    {"watcom::itoa", true, true, 3},
    {"watcom::ulltoa", true, true, 3},
    {"watcom::prtfUtoa", true, true, 3},
    {"watcom::U8D", true, true},
    // The environment, the time zone and the way out.
    {"watcom::mbterm", true, true, 1},
    {"watcom::mbsinc", true, true, 1},
    {"watcom::mbclen", true, true, 1},
    {"watcom::mbccpy", true, true, 2},
    {"watcom::mbccmp", true, true, 2},
    {"watcom::mbsnextc", true, true, 1},
    {"watcom::mbvtop", true, true, 2},
    {"watcom::mbbtrail", true, true, 1},
    {"watcom::mbclegal", true, true, 1},
    {"watcom::mbclower", true, true, 1},
    {"watcom::mbcupper932", true, true, 1},
    {"watcom::mbctoupper", true, true, 1},
    {"watcom::mbsupr", true, true, 1},
    {"watcom::mbcicmp", true, true, 2},
    {"watcom::mbsnicmp", true, true, 3},
    {"watcom::getenv", true, true, 1},
    {"watcom::wcstombs", true, true, 3},
    {"watcom::tzNumber", true, true, 2},
    {"watcom::tzName", true, true, 3},
    {"watcom::tzRule", true, true, 2},
    {"watcom::tzWindows", true, true},
    {"watcom::tzDaylight", true, true},
    {"watcom::tzset", true, true},
    {"watcom::callRoutine", true, true, 1},
    {"watcom::FiniRtns", true, true, 2},
    {"watcom::DoneExceptionHandler", true, true},
    {"watcom::freeEnvironment", true, false, 0, false, kStirred},
    {"watcom::exit_", true, false, 1, false, kStirred},
    {"watcom::_exit", true, false, 1, false, kStirred},
    {"watcom::exit", true, false, 1, false, kStirred},
    {"watcom::atexit", true, true, 1},
    {"watcom::EnterWVIDEO", true, true, 2},
    {"watcom::fatalWrite", true, false, 2, false, kStirred},
    {"watcom::fatal_runtime_error", true, false, 2, false, kStirred},
    {"watcom::sleep", true, true, 1},
    {"watcom::getpid", true, true},
    {"watcom::stackavail", true, true},
    {"watcom::GRO", true, true},
    {"watcom::hexDigit", true, true, 1},
    {"watcom::fullpath", true, true, 3, false, kStirred},
    {"watcom::tmpdir", true, true, 0, false, kStirred},
    {"watcom::MkTmpFile", true, true, 2, false, kStirred},
    // setenv and the heap's growing.
    {"watcom::mbslen", true, true, 1},
    {"watcom::mbtowc", true, true, 3},
    {"watcom::mbstowcs", true, true, 3},
    {"watcom::wcslen", true, true, 1},
    {"watcom::memcpy", true, true, 3},
    {"watcom::wcscpy", true, true, 2},
    {"watcom::towupper", true, true, 1},
    {"watcom::memmove", true, true, 3},
    {"watcom::msize", true, true, 1},
    {"watcom::HeapManager_expand", true, true, 4, false, kStirred},
    {"watcom::nexpand", true, true, 2, false, kStirred},
    {"watcom::nexpandFailed", true, true},
    {"watcom::nrealloc", true, true, 2, false, kStirred},
    {"watcom::findenv", true, true, 2, false, kStirred},
    {"watcom::setenv_", true, true, 3, false, kStirred},
    {"watcom::wfindenv", true, true, 2, false, kStirred},
    {"watcom::wsetenv", true, true, 3, false, kStirred},
    {"watcom::wputenvFind", true, true, 2, false, kStirred},
    {"watcom::wputenv", true, true, 1, false, kStirred},
    {"watcom::create_wide_environment", true, false, 0, false, kStirred},
    {"watcom::setenv", true, true, 3, false, kStirred},
    // Formatted input.
    {"watcom::cget_string", true, true, 1},
    {"watcom::uncget_string", true, true, 2},
    {"watcom::vsscanf", true, true, 3},
    {"watcom::sscanf", true, true},
    {"watcom::Strtold", true, true, 3, true},
    {"watcom::strtod", true, true, 2, true},
    {"watcom::cgetc", true, true, 1},
    {"watcom::uncget", true, true, 2},
    {"watcom::scnf", true, true, 3},
    {"watcom::get_opt", true, true, 2},
    {"watcom::scan_white", true, true, 1},
    {"watcom::scan_char", true, true, 2},
    {"watcom::scan_string", true, true, 2},
    {"watcom::report_scan", true, true, 3},
    {"watcom::makelist", true, true, 2},
    {"watcom::scan_arb", true, true, 3},
    {"watcom::scan_float", true, true, 2},
    {"watcom::scan_int", true, true, 4},
    {"watcom::radix_value", true, true, 1},
    {"watcom::chktrm", true, true, 1},
    {"watcom::ZBuf2LD", true, true, 2},
    {"watcom::normalize", true, true},
    {"watcom::scalePowers", true, true, 2, true},
    {"watcom::scaleEnd", true, true},
    {"watcom::scaleBy", true, true, 2, true},
    {"watcom::LDScale10x", true, true, 2, true},
    {"watcom::FDFS", true, true, 2},
    {"watcom::U8M", true, true, 4},
    // The console.
    {"watcom::NTRealKey", true, true, 1},
    {"watcom::openConsoles", true, true},
    {"watcom::NTConsoleInput", true, true},
    {"watcom::NTConsoleOutput", true, true},
    {"watcom::putch", true, true, 1},
    {"watcom::kbhit", true, true},
    {"watcom::do_getch", true, true, 1},
    {"watcom::getch", true, true},
    {"watcom::getche", true, true},
    // Structured exception handling.
    {"watcom::TryInit", true, true},
    {"watcom::TryHandler", true, true},
    {"watcom::TryScope", true, true},
    {"watcom::TryOuter", true, true},
    {"watcom::TryDone", true, true},
    {"watcom::Except", true, true},
    {"watcom::global_unwind", true, true},
    {"watcom::local_unwind", true, true},
    {"watcom::RtlUnwindThunk", true, true},
    // The start.
    {"watcom::startHook", true, true},
    {"watcom::InitRtns", true, true, 1},
    {"watcom::stackLimits", true, true, 2},
    {"watcom::calloc", true, true, 2, false, kStirred},
    {"watcom::initThread", true, true, 1},
    {"watcom::initThreadData", true, true, 1, false, kStirred},
    {"watcom::initPOSIXHandles", true, true, 0, false, kStirred},
    {"watcom::moduleWideName", true, true, 3, false, kStirred},
    {"watcom::copyCommandLine", true, true, 1, false, kStirred},
    {"watcom::copyWideCommandLine", true, true, 1, false, kStirred},
    {"watcom::InitExceptionHandling", true, true, 1},
    {"watcom::NTInit", true, true, 3, false, kStirred},
    {"watcom::NTMain", true, true, 2, false, kStirred},
    {"watcom::callMain", true, true},
    {"watcom::mainStart", true, true, 0, false, kStirred},
    {"watcom::cstart_", true, true, 0, false, kStirred},
    // The maths' error report, and the FDIV bug's atan2.
    {"watcom::get_std_stream", true, true, 1},
    {"watcom::printMathError", true, true, 2},
    {"watcom::callMatherr", true, true, 1, true},
    {"watcom::reportError", true, true, 3, true},
    {"watcom::reporterror", true, true, 2, true},
    // Its heap (native_watcom_heap.cpp).
    {"watcom::nmalloc", true, true, 1, false, kStirred},
    {"watcom::nfree", true, false, 1, false, kStirred},
    {"watcom::MemAllocator", true, true, 3, false, kStirred},
    {"watcom::MemFree", true, false, 3, false, kStirred},
    {"watcom::LinkUpNewNHeap", true, true, 1, false, kRestored},
    {"watcom::AdjustAmount", true, true, 1},
    {"watcom::CreateNewNHeap", true, true, 1, false, kStirred},
    {"watcom::ExpandDGROUP", true, true, 1, false, kStirred},
    {"watcom::nmemneed", true, true, 1},
    {"watcom::nheapshrink", true, true, 0, false, kStirred},
    {"watcom::ReturnMemToSystem", true, true, 1, false, kRestored},
    {"watcom::ReleaseNHeap", true, true, 1, false, kRestored},
    // The game's MMX code (native_mix.cpp): the SND library's routines keep
    // every register; the memmove's copy leaves eax, edx and ebx where it got
    // to, and keeps ecx.  The MMX registers are compared for all.
    {"mix::copySwapped", true, true},
    {"mix::copy", true, true},
    {"mix::addMono8", true, true},
    {"mix::addStereo8", true, true},
    {"mix::addMono16", true, true},
    {"mix::addStereo16", true, true},
    {"mix::outputMono8", true, true},
    {"mix::outputStereo8", true, true},
    {"mix::outputStereo16", true, true},
    {"mix::clear", true, true},
    {"mix::finished", true, true},
    {"mix::outputMono16", true, true},
    {"mix::interpolateMono8", true, true},
    {"mix::interpolateStereo8", true, true},
    {"mix::interpolateMono16", true, true},
    {"mix::interpolateStereo16", true, true},
    {"mix::xaMono", true, true},
    {"mix::xaStereo", true, true},
};

const Promise* promiseOf(const char* name)
{
    for (const Promise& p : kPromises)
        if (std::strcmp(p.name, name) == 0)
            return &p;
    return nullptr;
}

/* A float's bits of every kind: ordinary, tiny, huge, denormal, zero,
 * infinite, NaN quiet and signalling. */
x86::reg32 anyFloatBits(std::uint64_t r)
{
    const x86::reg32 low = x86::reg32(r >> 8);
    switch (r % 8)
    {
    case 0: return low;
    case 1: return (low & 0x80000000) | (low & 0x7fffff);              // zero or denormal
    case 2: return (low & 0x80000000) | 0x7f800000;                    // infinite
    case 3: return 0x7f800000 | (low & 0x3fffff) | 1;                  // signalling NaN
    case 4: return (low & 0x80000000) | 0x7fc00000 | (low & 0x3fffff); // quiet NaN
    default:
    {
        const float f = float(int(low % 20001) - 10000) * 0.37f;
        x86::reg32 bits;
        std::memcpy(&bits, &f, 4);
        return bits;
    }
    }
}

/* A Windows error, as GetLastError has one: the ones __set_errno_dos looks
 * for, one of the first 20 it has a table for, one past them, or anything. */
x86::reg32 anError(x86::reg32 r)
{
    static const x86::reg32 named[] = {0x7b, 0xce, 0xb7};
    switch (r % 4)
    {
    case 0: return named[(r >> 2) % 3];
    case 1: return r >> 8;
    default: return (r >> 8) % 0x16;
    }
}

/* A byte of a file: anything, but a '\r', a '\n' or a ^Z often, which text
 * streams make something of. */
x86::reg8 fileByte(std::uint64_t bits)
{
    return bits % 6 == 0 ? x86::reg8("\r\n\x1a"[(bits >> 8) % 3]) : x86::reg8(bits >> 16);
}

}  // namespace

namespace win32
{

/* A guest memory of its own, and its own log of calls out. */
class CheckApp : public WinApplication
{
public:
    CheckApp(bool generated) : WinApplication("game_checks", 0, {}), generated(generated) {}
    bool generated;
    std::vector<Call> calls;

    x86::reg32& word(x86::reg32 address) { return reinterpret_cast<x86::reg32&>(*&getMemory<x86::reg8>(address)); }

    x86::reg32 random(x86::reg32 id)
    {
        return x86::reg32(mix(g_seed ^ (std::uint64_t(calls.size()) << 32) ^ id));
    }

    std::uint64_t random64(x86::reg32 id)
    {
        return mix(g_seed ^ (std::uint64_t(calls.size()) << 32) ^ id ^ 0x5555);
    }

    /* Text of `length` at `at`, made from `r`. */
    void text(x86::reg32 at, x86::reg32 length, std::uint64_t r)
    {
        for (x86::reg32 i = 0; i < length; ++i)
        {
            r = mix(r);
            getMemory<x86::reg8>(at + i) = x86::reg8(' ' + r % 95);
        }
        getMemory<x86::reg8>(at + length) = 0;
    }

    /* Samples into a buffer, made from `r`: at most kSlotSize bytes, and only
     * into memory that is compared. */
    void samples(x86::reg32 at, x86::reg32 count, std::uint64_t r)
    {
        count = std::min<x86::reg32>(count, 0x800);
        if (!((at >= kHeap && at + 4 * count <= kHeapEnd) || (at >= kHarness && at + 4 * count <= kHarnessEnd)
              || at + 4 * count <= kLowEnd))
            return;
        for (x86::reg32 i = 0; i < count; ++i)
        {
            r = mix(r);
            word(at + 4 * i) = anyFloatBits(r);
        }
    }

    void standIn(const StandIn& s, x86::reg32 id, x86::CPU& cpu)
    {
        const x86::reg32 esp = cpu.esp;
        std::vector<x86::reg32> words;
        const x86::reg32 registers[] = {cpu.eax, cpu.edx, cpu.ebx, cpu.ecx};
        for (unsigned i = 0; i < s.registers; ++i)
            words.push_back(registers[i]);
        for (unsigned i = 0; i < s.words; ++i)
            words.push_back(word(esp + 4 + 4 * i));
        if (s.through >= 0)
        {
            const x86::reg32 from = s.through < 4 ? registers[s.through] : word(esp + 4 + 4 * (s.through - 4));
            // What a wild pointer would point at: nothing of the guest's.
            for (unsigned i = 0; i < s.deref; ++i)
                words.push_back(from + 4 * i < kMemory - 4 ? word(from + 4 * i) : 0xbad0bad0);
        }
        if (s.formats != 0)
        {
            const x86::reg32 format = s.formats == 1 ? registers[1] : word(esp + 4);
            const x86::reg32 arguments = s.formats == 1 ? word(registers[2]) : esp + 8;
            unsigned count = 0;
            for (x86::reg32 at = format; at < format + 0x400 && getMemory<char>(at) != 0; ++at)
                if (getMemory<char>(at) == '%')
                {
                    if (getMemory<char>(at + 1) == '%')
                        ++at;
                    else
                        ++count;
                }
            for (unsigned i = 0; i < count && i < 8; ++i)
                words.push_back(word(arguments + 4 * i));
        }
        if (s.effect == kCompare)
            words.push_back(esp);  // where the runtime's frame has it
        calls.push_back({id, words});
        if (long(calls.size()) - 1 == g_stopAt)
        {
            cpu.terminate = true;
            return;
        }
        const x86::reg32 r = random(id);
        x86::reg32 result = r;
        switch (s.effect)
        {
        case kText:
            result = r % 0x100;
            text(cpu.eax, result, random64(id));
            break;
        case kPrint:
            result = r % 0x50;  // within the caller's buffer (the speech's name's, 0x64, with its ending)
            if (word(0x30e004) == 0x4c42414c)
                result = r % 0x14;  // port::hud's label rows: a buffer of 0x18 below its locals
            if (words[1] == 0x537330)
                result = r % 0x10;  // "%sp": a car's folder's name, into 0x14 bytes (raceload::loadRaceCar)
            if (words[1] == 0x53b7f4)
                result = r % 0x0c;  // port::hud's percent text, into 0x10 bytes
            if (words[1] == 0x53b820)
            {
                // port::hud's row number, into 4 bytes: a digit, which the real text function can draw.
                getMemory<char>(words[0]) = char('1' + (r >> 4) % 9);
                getMemory<char>(words[0] + 1) = 0;
                result = 1;
                break;
            }
            if (words[1] == 0x53b8b0)
            {
                // port::hud's " of 8", into the same buffer: the text the real text function can draw.
                getMemory<char>(words[0]) = ' ';
                getMemory<char>(words[0] + 1) = 'o';
                getMemory<char>(words[0] + 2) = 'f';
                getMemory<char>(words[0] + 3) = ' ';
                getMemory<char>(words[0] + 4) = char('1' + (r >> 4) % 9);
                getMemory<char>(words[0] + 5) = 0;
                result = 5;
                break;
            }
            text(words[0], result, random64(id));
            break;
        case kLoad:
            result = r % 8 != 0 ? 0x7700 + (r >> 8) % 0x10 * 0x10 : 0;
            break;
        case kLookUp:
        {
            const std::string name = &getMemory<char>(words[1]);
            result = g_lookupFails && r % 16 == 0 ? 0 : namedStandIn(name);
            break;
        }
        case kVersion:
            result = r % 6 != 0 ? 0x100003 : r % 2 ? 0x100002 : r;
            break;
        case kAbout:
            result = r % 6 != 0 ? kAboutRecord : 0;
            break;
        case kBox:
            result = r % 3 != 0 ? 1 : 2;
            break;
        case kMainThread:
            result = r & 1;
            break;
        case kAllocate:
        {
            // A slot no one has, if the size fits one: the first free from
            // one picked at random.
            const x86::reg32 taken = word(kSlotsTaken);
            result = 0;
            if (r % 25 != 0 && words[1] != 0 && words[1] <= kSlotSize && taken != 0xff)
            {
                x86::reg32 slot = (r >> 8) % 8;
                while (taken >> slot & 1)
                    slot = (slot + 1) % 8;
                word(kSlotsTaken) = taken | 1u << slot;
                result = kHeap + kSlotSize * slot;
                samples(result, kSlotSize / 4, random64(id));
            }
            break;
        }
        case kFree:
        {
            // Only where a slot VirtualAlloc gave starts.
            const x86::reg32 slot = (words[0] - kHeap) / kSlotSize;
            result = 0;
            if (r % 25 != 0 && words[0] >= kHeap && words[0] < kHeapEnd && (words[0] - kHeap) % kSlotSize == 0
                && (word(kSlotsTaken) >> slot & 1))
            {
                word(kSlotsTaken) &= ~(1u << slot);
                result = 1;
            }
            break;
        }
        case kDecode:
        {
            // the buffer: the third argument, or the record's last word
            const x86::reg32 buffer = s.deref ? words.back() : words[2];
            const x86::reg32 count = s.deref ? words[1] : words[0];
            samples(buffer, count, random64(id));
            break;
        }
        case kFilter:
            result = r % 3 != 0 ? r | 1 : 0;
            break;
        case kLock:
            result = r % 2 != 0 ? 0x630000 + (r >> 8) % 0x100 * 0x10 : 0;
            break;
        case kSucceed:
            result = r % 5 != 0 ? 1 : 0;
            break;
        case kSomething:
            result = r | 1;
            break;
        case kString:
            result = kStrings + 0x40 * (r % 64);
            break;
        case kRecord:
            result = 0xc00000 + 0x40 * (r % 64);
            break;
        case kSoundChunk:
            result = r % 4 == 0 ? 0 : 0xc2c000 + 0x40 * (0x10 + (r >> 8) % 0x10);
            break;
        case kLastError:
            result = anError(r);
            break;
        case kAttributes:
            result = r % 5 != 0 ? (r >> 8) & 0x1ff : 0xffffffff;
            break;
        case kDirectory:
        {
            // The path as long as it is, up to past MAX_PATH; into the
            // buffer if it fits, else how much it would take; or nothing.
            const x86::reg32 length = 1 + (r >> 8) % (g_shortDirectory ? 0x100 : 0x120);
            if (r % 9 == 0)
            {
                result = 0;
            }
            else if (length < words[0])
            {
                text(words[1], length, random64(id));
                result = length;
            }
            else
            {
                result = length + 1;
            }
            break;
        }
        case kDiskFree:
            result = r % 5 != 0 ? 1 : 0;
            if (result != 0)
                for (unsigned i = 1; i <= 4; ++i)
                    word(words[i]) = random(id + i);
            break;
        case kFind:
            result = r % 5 != 0 ? 0x7f00 + (r >> 8) % 0x100 : 0xffffffff;
            if (result != 0xffffffff)
                for (x86::reg32 i = 0; i < 0x140; i += 4)
                    word(words[1] + i) = i == 0 ? (r >> 8) & 0x1ff : random(id + i);
            break;
        case kLocalTime:
        {
            // Year, month, day of the week, day, hour, minute, second,
            // millisecond: in range mostly, anything now and then.
            static const x86::reg32 ranges[] = {200, 12, 7, 31, 24, 60, 60, 1000};
            for (unsigned i = 0; i < 8; ++i)
            {
                const x86::reg32 v = random(id + 3 * i);
                const x86::reg32 in = i == 0 ? 1900 + v % ranges[0] : (i == 1 || i == 3 ? 1 : 0) + v % ranges[i];
                getMemory<x86::reg16>(words[0] + 2 * i) = x86::reg16(r % 9 != 0 ? in : v >> 8);
            }
            break;
        }
        case kZone:
            if (r % 3 == 0)
            {
                word(kTimezone) = x86::reg32(x86::sreg32(random(id + 1) % 100801) - 50400);
                word(kDstAdjust) = random(id + 2) % 4 != 0 ? 3600 : random(id + 3) % 7201;
            }
            break;
        case kBankAdd:
            if (words[0] < kMemory - 4)
                word(words[0]) = r % 24;
            result = r % 5 == 0 ? 0xfffffff7 : r % 3 == 0 ? 7 : 0;
            break;
        case kMathResult:
            cpu.fpu.count += 1;
            cpu.fpu.st(0) = r % 5 != 0 ? x86::Float(double(x86::sreg32(r)) / 7.0) : x86::Float(r % 2 ? INFINITY : NAN);
            break;
        case kSeek:
            // A place: near the start often, for fseek within a buffer.
            result = r % 6 == 0 ? 0xffffffff : r % 3 == 0 ? (r >> 8) % 0x800 : r >> 4;
            break;
        case kCharacter:
            result = r % 6 != 0 ? (r >> 8) & 0xff : 0xffffffff;
            break;
        case kOpenFile:
            result = r % 5 != 0 ? 0x7000 + (r >> 8) % 0x100 * 4 : 0xffffffff;
            break;
        case kEvent:
            result = r % 3 != 0 ? 0x7400 + (r >> 8) % 0x100 * 4 : 0;
            break;
        case kHandle:
            result = r % 8 != 0 ? (r >> 8) % 20 : (r >> 3) % 2 ? 20 + (r >> 8) % 3 : 0xffffffff;
            break;
        case kReallocate:
            // Where they were, moved; or nothing.
            result = r % 5 != 0 ? kGrownTable : 0;
            if (result != 0)
                for (x86::reg32 i = 0; i < std::min<x86::reg32>(words[1], 0x100); i += 4)
                    word(kGrownTable + i) = word(words[0] + i);
            break;
        case kTmpName:
            text(words[0], 8 + r % 24, random64(id));
            break;
        case kWideChar:
            // (code page, flags, the character, 1, the buffer, 2, 0, 0): a
            // byte or two, or none.
            result = r % 8 == 0 ? 0 : 1 + (r >> 3) % 2;
            for (x86::reg32 i = 0; i < result; ++i)
                getMemory<x86::reg8>(words[4] + i) = x86::reg8(r >> (8 + 8 * i));
            break;
        case kZoneInfo:
        {
            // A TIME_ZONE_INFORMATION: a bias, the two names, their dates and
            // biases; unknown, standard, daylight, or failed.
            static const x86::reg32 results[] = {0, 1, 2, 2, 1, 0xffffffff};
            result = results[r % 6];
            word(words[0]) = x86::reg32(x86::sreg32((r >> 4) % 1441) - 720);
            for (const x86::reg32 name : {words[0] + 4, words[0] + 0x58})
            {
                const x86::reg32 length = random(id + name) % 33;
                for (x86::reg32 i = 0; i < 32; ++i)
                    getMemory<x86::reg16>(name + 2 * i) =
                        x86::reg16(i >= length ? 0 : random(id + name + i) % 5 == 0 ? random(id + i) : 'A' + i % 26);
            }
            for (x86::reg32 i = 0x44; i < 0x54; i += 4)
                word(words[0] + i) = random(id + i);
            for (x86::reg32 i = 0x98; i < 0xa8; i += 4)
                word(words[0] + i) = random(id + i);
            word(words[0] + 0x54) = r % 3 == 0 ? 0 : x86::reg32(-60);
            word(words[0] + 0xa8) = r % 4 == 0 ? 0 : x86::reg32(-60);
            break;
        }
        case kUpper:
            // (the buffer, how many): ASCII's small letters made capitals.
            for (x86::reg32 i = 0; i < words[1] && i < 0x100; ++i)
            {
                const x86::reg8 c = getMemory<x86::reg8>(words[0] + i);
                if (c >= 'a' && c <= 'z')
                    getMemory<x86::reg8>(words[0] + i) = x86::reg8(c - 0x20);
            }
            result = words[1];
            break;
        case kMultiByte:
            // (code page, flags, the bytes, how many, the buffer, 1)
            result = r % 8 == 0 ? 0 : 1;
            if (result != 0)
                getMemory<x86::reg16>(words[4]) = x86::reg16(r % 3 == 0 ? r >> 8 : 'a' + (r >> 8) % 26);
            break;
        case kFullPath:
        {
            // (the path, the size, the buffer, &the file's part): the path
            // as long as it is, into the buffer if it fits, else how much it
            // would take; or nothing.
            const x86::reg32 length = 1 + (r >> 8) % 0x120;
            if (r % 9 == 0)
                result = 0;
            else if (length < words[1])
            {
                text(words[2], length, random64(id));
                word(words[3]) = words[2] + (r >> 16) % length;
                result = length;
            }
            else
                result = length + 1;
            break;
        }
        case kConsoleInput:
        {
            // (the console, the record, 1, &how many): a key pressed or let
            // go mostly -- shift, control or alt alone now and then, an
            // extended key, none of a character -- or another event; none
            // now and then, or FALSE.
            const bool failed = r % 9 == 0;
            const x86::reg32 count = failed || r % 5 == 0 ? 0 : 1;
            result = failed ? 0 : 1;
            if (!failed)
                word(words[3]) = count;
            if (count != 0)
            {
                const x86::reg32 record = words[1];
                getMemory<x86::reg16>(record) = x86::reg16((r >> 4) % 6 != 0 ? 1 : 2);
                word(record + 4) = (r >> 7) % 4 != 0 ? 1 : 0;
                getMemory<x86::reg16>(record + 8) = x86::reg16(1 + (r >> 9) % 3);
                getMemory<x86::reg16>(record + 0xa) =
                    x86::reg16((r >> 11) % 4 == 0 ? 0x10 + (r >> 13) % 3 : 0x41 + (r >> 13) % 26);
                getMemory<x86::reg16>(record + 0xc) = x86::reg16(random(id + 1) & 0xff);
                getMemory<x86::reg16>(record + 0xe) = x86::reg16((r >> 16) % 5 == 0 ? 0 : 'a' + (r >> 18) % 26);
                word(record + 0x10) = (r >> 23) % 5 == 0 ? 0x100 : random(id + 2) & 0x1f;
            }
            break;
        }
        case kModuleName:
        {
            // (the module, the buffer, its size): a path as long as it is,
            // cut to the size, or nothing; wide for GetModuleFileNameW.
            const bool wide = s.address == 0x534548;
            const x86::reg32 length = std::min<x86::reg32>(1 + (r >> 8) % 0x60, words[2] - 1);
            result = r % 9 == 0 || words[2] == 0 ? 0 : length;
            for (x86::reg32 i = 0; result != 0 && i <= length; ++i)
            {
                const x86::reg8 c = i == length ? 0 : x86::reg8('A' + random(id + i) % 26);
                if (wide)
                    getMemory<x86::reg16>(words[1] + 2 * i) = c;
                else
                    getMemory<x86::reg8>(words[1] + i) = c;
            }
            break;
        }
        case kVersionInfo:
            result = ((r >> 8) & 0xffff) << 16 | (r % 4) << 8 | (3 + (r >> 4) % 4);
            break;
        case kStdHandle:
            result = r % 6 == 0 ? 0 : r % 6 == 1 ? 0xffffffff : 0x7000 + (r >> 8) % 0x100 * 4;
            break;
        case kQuery:
            // (the address, the buffer, its size): the region it is in --
            // its base, where the allocation starts, its size.
            word(words[1]) = words[0] & 0xfffff000;
            word(words[1] + 4) = (words[0] & 0xfff00000) - (r >> 8) % 0x100 * 0x1000;
            word(words[1] + 8) = 4;
            word(words[1] + 0xc) = 0x1000 * (1 + (r >> 16) % 0x40);
            word(words[1] + 0x10) = 0x1000;
            word(words[1] + 0x14) = 4;
            word(words[1] + 0x18) = 0x20000;
            result = 0x1c;
            break;
        case kCommandLine:
            result = s.address == 0x5344f4 ? kBuffers + 0x200 * 14 : r % 5 == 0 ? 0 : kBuffers + 0x200 * 14 + 0x100;
            break;
        case kMatherrSay:
            result = r % 4 == 0 ? 1 : 0;
            break;
        case kBlock:
            if (word(0x30e000) == 0x57454956)
            {
                // The view's glyph resource (view.inc): blocks that follow one another, as large as asked
                // (edx), so that what it moves from one to the next does not run over itself.
                result = word(0x30e004);
                word(0x30e004) += (cpu.edx + 15) & ~x86::reg32(15);
                break;
            }
            result = r % 6 == 0 ? 0 : kBlocks + 0x400 * ((r >> 8) % 8);
            break;
#define CONTROLS_EFFECT_CASES
#include "controls_input.inc"
#include "controls_devices.inc"
#include "controls_race.inc"
#include "controls_pipeline.inc"
#include "controls_queue.inc"
#undef CONTROLS_EFFECT_CASES
#define EA_EFFECT_CASES
#include "ea_threads.inc"
#include "ea_memory.inc"
#include "ea_files.inc"
#include "ea_support.inc"
#include "ea_net.inc"
#include "ea_formats.inc"
#undef EA_EFFECT_CASES
#define LOAD_EFFECT_CASES
#include "load_track.inc"
#include "load_cardb.inc"
#include "load_carmodels.inc"
#include "load_saves.inc"
#undef LOAD_EFFECT_CASES
#define FE_EFFECT_CASES
#include "frontend_glue.inc"
#include "frontend_draw.inc"
#undef FE_EFFECT_CASES
#define TEXT_EFFECT_CASES
#include "text.inc"
#undef TEXT_EFFECT_CASES
#define BUTTONS_EFFECT_CASES
#include "buttons.inc"
#undef BUTTONS_EFFECT_CASES
#define MENUS_EFFECT_CASES
#include "menus.inc"
#undef MENUS_EFFECT_CASES
#define LOOP_EFFECT_CASES
#include "loop.inc"
#undef LOOP_EFFECT_CASES
#define DIALOGS_EFFECT_CASES
#include "dialogs.inc"
#undef DIALOGS_EFFECT_CASES
#define VIEW_EFFECT_CASES
#include "view.inc"
#undef VIEW_EFFECT_CASES
#define HUD_EFFECT_CASES
#include "hud.inc"
#undef HUD_EFFECT_CASES
#define CONV_EFFECT_CASES
#include "conv.inc"
#undef CONV_EFFECT_CASES
#define RACESTART_EFFECT_CASES
#include "racestart.inc"
#undef RACESTART_EFFECT_CASES
#define RACELOAD_EFFECT_CASES
#include "raceload.inc"
#undef RACELOAD_EFFECT_CASES
#define TRACKDATA_EFFECT_CASES
#include "trackdata.inc"
#undef TRACKDATA_EFFECT_CASES
#define PHYSICS_EFFECT_CASES
#include "physics.inc"
#undef PHYSICS_EFFECT_CASES
#define HANDLING_EFFECT_CASES
#include "handling.inc"
#undef HANDLING_EFFECT_CASES
#define DRIVETRAIN_EFFECT_CASES
#include "drivetrain.inc"
#undef DRIVETRAIN_EFFECT_CASES
#define CARSTATE_EFFECT_CASES
#include "carstate.inc"
#undef CARSTATE_EFFECT_CASES
#define TRACK_EFFECT_CASES
#include "track.inc"
#undef TRACK_EFFECT_CASES
#define COLLIDE_EFFECT_CASES
#include "collide.inc"
#undef COLLIDE_EFFECT_CASES
#define AIDRIVE_EFFECT_CASES
#include "aidrive.inc"
#undef AIDRIVE_EFFECT_CASES
#define AI_EFFECT_CASES
#include "ai_race.inc"
#undef AI_EFFECT_CASES
#define PURSUIT_EFFECT_CASES
#include "pursuit.inc"
#undef PURSUIT_EFFECT_CASES
#define AIPLAN_EFFECT_CASES
#include "aiplan.inc"
#undef AIPLAN_EFFECT_CASES
#define OBJ3D_EFFECT_CASES
#include "obj3d.inc"
#undef OBJ3D_EFFECT_CASES
        case kRectangle:
            // A screen's, mostly: 640 by 480 to 1600 by 1200, or any size.
            word(cpu.edx) = r % 64;
            word(cpu.ebx) = (r >> 6) % 64;
            word(cpu.ecx) = 320 + (r >> 12) % 1300;
            word(word(esp + 4)) = r % 8 == 0 ? random(id + 1) : 240 + (r >> 20) % 1000;
            break;
        case kFloatBack:
            cpu.fpu.count += 1;
            cpu.fpu.st(0) = x86::Float(double(x86::sreg32(r)) / 7.0);
            break;
        case kTickWait:
            word(0x79c1a4) += 1 + r % 2;
            // (The menu loop waits out a window sent to the background by this: it comes back now and then.)
            if (word(0x559444) != 0 && r % 5 == 0)
                word(0x559444) = 0;
            break;
        case kScreenName:
        {
            // (And the car screens' the menu loop's settings look for: showcase, interior, history, compare.)
            static const x86::reg32 names[] = {0, 0x53793c, 0x537944, 0x537950, 0x53795c, 0x539084, 0x539090, 0x53909c, 0x5390a4};
            result = names[r % 9];
            break;
        }
        case kConsoleMode:
            result = r % 7 != 0 ? 1 : 0;
            if (result != 0)
                word(words[1]) = (r >> 8) & 0x1ff;
            break;
        case kScanDouble:
        {
            const std::uint64_t bits = random64(id);
            word(words[1]) = x86::reg32(bits);
            word(words[1] + 4) = x86::reg32(bits >> 32);
            break;
        }
        case kFloat:
        {
            // (the buffer, the arguments' pointer, the SPECS): a double's 8
            // bytes taken, text after the buffer's first byte -- a sign or
            // nothing -- and the parts: the sign's, the digits', zeros after.
            const x86::reg32 length = r % 0x20;
            word(words[1]) += 8;
            getMemory<x86::reg8>(words[0]) = r & 0x100 ? '-' : 0;
            text(words[0] + 1, length, random64(id));
            word(words[2] + 0x20) = r >> 8 & 1;
            word(words[2] + 0x28) = length;
            word(words[2] + 0x2c) = (r >> 9) % 3;
            break;
        }
        case kRead:
        case kTransfer:
        {
            // How many: as many as asked for mostly, fewer, none; read
            // ones written into the buffer.
            const x86::reg32 asked = std::min<x86::reg32>(words[2], 0x10000);
            const x86::reg32 count = r % 3 != 0 ? asked : (r >> 4) % 3 == 0 ? 0 : (r >> 8) % (asked + 1);
            const bool failed = r % 6 == 0;
            if (!failed && (s.effect == kRead || s.address == kWindowsStdin))
            {
                std::uint64_t bits = random64(id);
                for (x86::reg32 i = 0; i < count; ++i)
                    getMemory<x86::reg8>(words[1] + i) = fileByte(bits = mix(bits));
            }
            if (s.effect == kRead)
            {
                if (!failed)
                    word(words[3]) = count;
                result = failed ? 0 : 1;
            }
            else
            {
                result = failed ? 0xffffffff : count;
            }
            break;
        }
        case kWrite:
        {
            const x86::reg32 asked = std::min<x86::reg32>(words[2], 0x10000);
            result = r % 6 != 0 ? 1 : 0;
            // All of it mostly; fewer, none now and then -- a disk full.
            if (result != 0)
                word(words[3]) = r % 4 != 0 ? words[2] : (r >> 4) % 3 == 0 ? 0 : (r >> 8) % (asked + 1);
            break;
        }
        case kFileType:
            result = (r >> 8) % 4;
            break;
        case kWindowed:
            result = r % 2 != 0 ? 0x77000 + (r >> 8) % 0x100 * 4 : 0;
            break;
        case kCompare:
            // The first dwords' difference, signed and as it wraps; the first
            // bytes' order; or anything.
            if (s.address == kByDword)
            {
                result = word(words[0]) - word(words[1]);
            }
            else if (s.address == kByByte)
            {
                const x86::reg8 a = getMemory<x86::reg8>(words[0]), b = getMemory<x86::reg8>(words[1]);
                result = a < b ? 0xffffffff : a > b ? 1 : 0;
            }
            else
            {
                result = r % 3 == 0 ? 0 : r % 3 == 1 ? r | 0x80000000 : (r >> 1) | 1;
            }
            break;
        case kNothing:
            // The view's glyph resource (view.inc): whether the cache file is there, half and half.
            if (word(0x30e000) == 0x57454956 && s.address == 0x4e0e60)
                result = (r >> 12) % 2 == 0 ? 0 : r | 1;
            break;
        }
        // What it may leave otherwise, it does now and then.
        const x86::reg32 more = random(id ^ 0xabcd);
        if (s.clobbers & kEax)
            cpu.eax = s.effect != kNothing || more % 4 != 0 ? result : cpu.eax;
        if ((s.clobbers & kEcx) && (more >> 2) % 4 != 0)
            cpu.ecx = more ^ 0x11111111;
        if ((s.clobbers & kEdx) && (more >> 4) % 4 != 0)
            cpu.edx = more ^ 0x22222222;
        if ((s.clobbers & kEbx) && (more >> 6) % 4 != 0)
            cpu.ebx = more ^ 0x33333333;
        cpu.esp = esp + 4 + (s.convention == kStdcall ? 4 * s.words : 0);
    }

    /* A function GetProcAddress gave: stdcall, its words from its name's @. */
    void named(x86::reg32 index, x86::CPU& cpu)
    {
        const std::string& name = g_names[index];
        const size_t at = name.rfind('@');
        const x86::reg32 words = at == std::string::npos ? 0 : x86::reg32(std::atoi(name.c_str() + at + 1)) / 4;
        Effect effect = kNothing;
        if (name == "_iSNDdllversion@0")
            effect = kVersion;
        else if (name == "_THRASH_about@0" || name == "_THRASH_lockwindow@0")
            effect = kAbout;  // a record or none: THRASH_about's, or as the lock's
        const StandIn s = {kNamed + 0x10 * index, name.c_str(), kStdcall, 0, words, kScratch, -1, 0, effect};
        standIn(s, s.address, cpu);
    }
};

CheckApp* g_running = nullptr;

WinApplication::WinApplication(const char*, x86::reg32, const std::vector<Section>&)
{
#ifdef _WIN32
    // Watched for writes, for the windows to be copied and compared only where written.
    void* memory = VirtualAlloc(nullptr, kMemory, MEM_RESERVE | MEM_COMMIT | MEM_WRITE_WATCH, PAGE_READWRITE);
    if (!memory)
        std::abort();
#else
    void* memory = mmap(nullptr, kMemory, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED)
        std::abort();
#endif
    m_memory = static_cast<x86::reg8*>(memory);
}

WinApplication::~WinApplication() {}

int WinApplication::s_traceApi = 0;
bool WinApplication::readTraceApi() { return false; }
void WinApplication::traceCall(const Method&, x86::reg32) {}
void WinApplication::yieldContext(const x86::CPU&) {}
void WinApplication::lockContext(const x86::CPU&) {}
void WinApplication::unlockContext(const x86::CPU&) {}
void Thread::sleep(x86::reg32) {}

/* Whether the function under test is a rewrite (port::): the generated side then runs the
 * game's code of another rewrite it calls, so the native side runs that rewrite's adapter, not
 * the stand-in a call by address would find. */
bool g_testedPort = false;
const x86::reg32 kStoodInBoth[] = {
#include "game_stood_in.inc"
    0};

void WinApplication::dynamicCallElsewhere(x86::reg32 address, x86::CPU& cpu)
{
    CheckApp* app = static_cast<CheckApp*>(this);
    if (address == 0)
    {
        // A pointer the game never set, as the game calls it: the end of it.
        app->calls.push_back({0, {}});
        cpu.terminate = true;
        return;
    }
    const auto stoodInBoth = [](x86::reg32 a) {
        for (const x86::reg32 s : kStoodInBoth)
            if (s == a)
                return true;
        return false;
    };
    if (g_testedPort && !app->generated)
        for (const Tested& t : kTested)
            if (t.address == address && std::strncmp(t.name, "port::", 6) == 0 && !stoodInBoth(address))
            {
                t.native(this, cpu);
                return;
            }
    for (const StandIn& s : kGameFunctions)
        if (s.address == address)
        {
            app->standIn(s, address, cpu);
            return;
        }
    if (address >= kStandIns && address < kStandIns + 0x10 * kPointerCount && (address & 0xf) == 0)
    {
        app->standIn(kPointers[(address - kStandIns) / 0x10], address, cpu);
        return;
    }
    if (address >= kNamed && address < kNamed + 0x10 * x86::reg32(g_names.size()) && (address & 0xf) == 0)
    {
        app->named((address - kNamed) / 0x10, cpu);
        return;
    }
    for (const Tested& t : kTested)
        if (t.address == address)
        {
            (app->generated ? t.generated : t.native)(this, cpu);
            return;
        }
    std::printf("a call to %08x, no stand-in there (%s, %zu calls)\n", unsigned(address),
                app->generated ? "generated" : "native", app->calls.size());
    std::exit(2);
}

void MemMap::usedRanges(std::vector<std::pair<x86::reg32, x86::reg32>>& ranges)
{
    ranges.assign(1, {0, kMemory});
}

}  // namespace win32

#define TEXT_HOOKS
#include "text.inc"
#undef TEXT_HOOKS
#define DIALOGS_HOOKS
#include "dialogs.inc"
#undef DIALOGS_HOOKS
#define HUD_HOOKS
#include "hud.inc"
#undef HUD_HOOKS
#define CONV_HOOKS
#include "conv.inc"
#undef CONV_HOOKS

/* The game's functions the generated code calls by address: their stand-ins. */
#define STAND_IN(a)                                                                      \
    void nfs3hp::Application::sub_##a(WinApplication* app, x86::CPU& cpu)               \
    {                                                                                    \
        app->dynamic_call(0x##a, cpu);                                                   \
    }
#include "game_called.inc"

namespace
{

using win32::CheckApp;

std::mt19937 g_random(1998);

x86::reg32 random32()
{
    return x86::reg32(g_random());
}

x86::reg32 below(x86::reg32 n)
{
    return x86::reg32(g_random() % n);
}

bool chance(int percent)
{
    return int(g_random() % 100) < percent;
}

x86::reg8* bytes(win32::WinApplication& app, x86::reg32 at)
{
    return &app.getMemory<x86::reg8>(at);
}

struct Counts
{
    unsigned cases = 0, stopped = 0, failed = 0;
};

const char* g_name = "";
unsigned g_case = 0;

bool report(Counts& counts, const char* what)
{
    if (++counts.failed <= 10)
        std::printf("  %s case %u: %s\n", g_name, g_case, what);
    return false;
}

/* The first byte of [from, to) where the two memories differ, or `to`. */
x86::reg32 firstDifference(win32::WinApplication& a, win32::WinApplication& b, x86::reg32 from, x86::reg32 to)
{
    if (to <= from || std::memcmp(bytes(a, from), bytes(b, from), to - from) == 0)
        return to;
    for (x86::reg32 at = from; at < to; ++at)
        if (*bytes(a, at) != *bytes(b, at))
            return at;
    return to;
}

constexpr x86::reg32 kPage = 0x1000;

/* The pages of a memory written since it was last asked with `reset`, added to
 * `pages`.  Copying and comparing every window, some megabytes, every case was what
 * the checks waited on: with all the machine's threads they got no further in a
 * second than with one, the memory's bandwidth being the limit. */
void writtenPages(win32::WinApplication& app, bool reset, std::vector<x86::reg32>& pages)
{
#ifdef _WIN32
    static std::vector<void*> found(kMemory / kPage);
    ULONG_PTR count = found.size();
    DWORD granularity = 0;
    x86::reg8* const base = bytes(app, 0);
    if (GetWriteWatch(reset ? WRITE_WATCH_FLAG_RESET : 0, base, kMemory, found.data(), &count, &granularity) != 0)
    {
        std::printf("GetWriteWatch failed: %lu\n", GetLastError());
        std::exit(2);
    }
    for (ULONG_PTR i = 0; i < count; ++i)
        pages.push_back(x86::reg32(static_cast<x86::reg8*>(found[i]) - base));
#else
    for (x86::reg32 page = 0; page < kMemory; page += kPage)
        pages.push_back(page);
#endif
}

/* What two memories alike in their windows when last copied may differ in: the
 * pages either has written since, in order. */
std::vector<x86::reg32>& pagesEitherWrote(win32::WinApplication& a, win32::WinApplication& b, bool reset)
{
    static std::vector<x86::reg32> pages;
    pages.clear();
    writtenPages(a, reset, pages);
    writtenPages(b, reset, pages);
    std::sort(pages.begin(), pages.end());
    pages.erase(std::unique(pages.begin(), pages.end()), pages.end());
    return pages;
}

/* The first byte of the window [from, to), at `pages`, where the two differ, or `to`. */
x86::reg32 firstDifferenceIn(win32::WinApplication& a, win32::WinApplication& b, x86::reg32 from, x86::reg32 to,
                             const std::vector<x86::reg32>& pages)
{
    for (auto page = std::lower_bound(pages.begin(), pages.end(), from & ~(kPage - 1));
         page != pages.end() && *page < to; ++page)
    {
        const x86::reg32 at = firstDifference(a, b, std::max(*page, from), std::min(*page + kPage, to));
        if (at != std::min(*page + kPage, to))
            return at;
    }
    return to;
}

/* `to`'s windows made `from`'s, where either was written since this was last done. */
void copyWindows(win32::WinApplication& to, win32::WinApplication& from)
{
    const std::vector<x86::reg32>& pages = pagesEitherWrote(to, from, true);
    for (const auto& w : kWindows)
    {
        for (auto page = std::lower_bound(pages.begin(), pages.end(), w.first & ~(kPage - 1));
             page != pages.end() && *page < w.second; ++page)
        {
            const x86::reg32 start = std::max(*page, w.first);
            const x86::reg32 end = std::min(*page + kPage, w.second);
            std::memcpy(bytes(to, start), bytes(from, start), end - start);
        }
    }
    // What was just copied is no news.
    static std::vector<x86::reg32> copied;
    copied.clear();
    writtenPages(to, true, copied);
}

x86::reg32 aString()
{
    return kStrings + 0x40 * below(64);
}

/* A Float's bits, as hex: the 80 of the x87's, or the double's. */
std::string floatBits(const x86::Float& f)
{
    const double value = f;
    std::uint64_t bits;
    std::memcpy(&bits, &value, 8);
    const unsigned words[] = {unsigned(bits >> 48), unsigned(bits >> 32 & 0xffff), unsigned(bits >> 16 & 0xffff),
                              unsigned(bits & 0xffff)};
    std::string text;
    char word[8];
    for (const unsigned w : words)
    {
        std::snprintf(word, sizeof word, "%04x", w);
        text += word;
    }
    return text;
}

/* A double of every kind: an integer and a half, a fraction, huge, tiny,
 * denormal, zero, infinite, NaN -- of either sign. */
double anyDouble()
{
    double value;
    switch (below(8))
    {
    case 0: { std::uint64_t bits = std::uint64_t(random32()) << 32 | random32(); std::memcpy(&value, &bits, 8); return value; }
    case 1: value = double(x86::sreg32(random32()) >> below(31)) + 0.5 * below(3); break;
    case 2: value = std::ldexp(double(random32()), int(below(140)) - 70); break;
    case 3: value = double(below(5)) * 0.25; break;
    case 4: value = std::ldexp(1.0 + double(below(1000)) / 1000.0, int(below(2100)) - 1075); break;
    case 5: value = chance(50) ? INFINITY : NAN; break;
    default: value = double(x86::sreg32(random32() >> below(32))) * 0.37; break;
    }
    return chance(50) ? -value : value;
}

x86::reg32 textString(x86::reg32 i)
{
    return kTexts + 0x100 * i;
}

x86::reg32 aText()
{
    return textString(below(32));
}

x86::reg32 aBuffer()
{
    return kBuffers + 0x200 * below(16);
}

/* The string functions' strings, made up again for each case: in pairs, the
 * second the first with a change -- a letter's case, a byte, the end moved --
 * of letters, digits, signs, spaces and bytes above 0x7f, as the case picks
 * its alphabet. */
void makeText(CheckApp& g)
{
    static const char* const alphabets[] = {"abcAB", "aA bB,;", "0123456789+- \t", "xyzXYZ\x80\xc1\xff",
                                            "the quick brown fox", "aaaa", " \t\n\v\f\r19-"};
    const char* alphabet = alphabets[below(7)];
    const x86::reg32 size = x86::reg32(std::strlen(alphabet));
    for (x86::reg32 i = 0; i < 32; i += 2)
    {
        const x86::reg32 length = chance(10) ? 0 : 1 + below(chance(70) ? 12 : 0xf0);
        x86::reg8 first[0x100], second[0x100];
        for (x86::reg32 j = 0; j < length; ++j)
            first[j] = chance(95) ? x86::reg8(alphabet[below(size)]) : x86::reg8(1 + below(255));
        first[length] = 0;
        std::memcpy(second, first, length + 1);
        x86::reg32 end = length;
        switch (below(6))
        {
        case 0: break;
        case 1: if (length) second[below(length)] ^= 0x20; break;
        case 2: if (length) second[below(length)] = x86::reg8(1 + below(255)); break;
        case 3: end = below(length + 1); second[end] = 0; break;
        case 4: if (length < 0xf0) { second[length] = x86::reg8(alphabet[below(size)]); second[length + 1] = 0; } break;
        default: if (length) second[length - 1] = x86::reg8(0x80 | random32()); break;
        }
        std::memcpy(&g.getMemory<x86::reg8>(textString(i)), first, 0x100);
        std::memcpy(&g.getMemory<x86::reg8>(textString(i + 1)), second, 0x100);
    }
    for (x86::reg32 at = kBuffers; at < kBuffers + 0x2000; at += 4)
        g.word(at) = chance(30) ? 0 : random32();
}

/* A copy of a string of kTexts into a buffer, for the functions that write. */
x86::reg32 copied(CheckApp& g, x86::reg32 from)
{
    const x86::reg32 buffer = aBuffer();
    std::memcpy(&g.getMemory<x86::reg8>(buffer), &g.getMemory<x86::reg8>(from), 0x100);
    return buffer;
}

/* The runtime's heap's miniheaps, in the list's order: each in a slot of
 * VirtualAlloc's. */
std::vector<x86::reg32> miniheaps(CheckApp& g)
{
    std::vector<x86::reg32> heaps;
    for (x86::reg32 heap = g.word(kHeapFirst); heap >= kHeap && heap < kHeapEnd && heaps.size() < 8;
         heap = g.word(heap + 8))
        heaps.push_back(heap);
    return heaps;
}

/* The miniheap a block is in, or 0. */
x86::reg32 heapHolding(CheckApp& g, x86::reg32 block)
{
    for (const x86::reg32 heap : miniheaps(g))
        if (heap <= block && block < heap + g.word(heap))
            return heap;
    return 0;
}

/* The blocks the heap gave the generated _nmalloc and __MemAllocator, for
 * the cases that give them back; one given back is off the list, whether the
 * case got that far or not. */
std::vector<x86::reg32> g_blocks;

x86::reg32 takeBlock()
{
    const size_t at = below(x86::reg32(g_blocks.size()));
    const x86::reg32 block = g_blocks[at];
    g_blocks.erase(g_blocks.begin() + at);
    return block;
}

/* An amount to ask the heap for: mostly a few bytes, up to most of a slot;
 * none, or more than there is. */
x86::reg32 anAmount()
{
    switch (below(10))
    {
    case 0: return chance(30) ? 0 : chance(50) ? 0xffffffd0 + below(0x10) : random32();
    case 1: return 1 + below(0x18000);
    case 2: case 3: return 1 + below(0x2000);
    default: return 1 + below(0x100);
    }
}

const Tested& testedNamed(const char* name)
{
    for (const Tested& t : kTested)
        if (std::strcmp(t.name, name) == 0)
            return t;
    std::abort();
}

/* The rover on a miniheap given back -- as __ReturnMemToSystem leaves it when
 * that was the only one -- is where the game would fault, its memory gone;
 * here it is not, and what the heap does with it next would only spoil it.
 * The heap starts afresh instead. */
void keepHeapSound(CheckApp& g)
{
    const x86::reg32 rover = g.word(kHeapRover);
    const std::vector<x86::reg32> heaps = miniheaps(g);
    if (rover == 0 || std::find(heaps.begin(), heaps.end(), rover) != heaps.end())
        return;
    for (const x86::reg32 at : {kHeapFirst, kHeapRover, kHeapLargestBefore, kHeapFreeRover})
        g.word(at) = 0;
    g_blocks.clear();
}

/* The heap as the game's use leaves it: before each case of a function of
 * the heap's, a few blocks taken or given back by the generated _nmalloc and
 * _nfree, two dozen held at most -- miniheaps full, empty and in between. */
void stirHeap(CheckApp& g)
{
    const Tested& nmalloc = testedNamed("watcom::nmalloc");
    const Tested& nfree = testedNamed("watcom::nfree");
    for (x86::reg32 i = below(6); i > 0; --i)
    {
        x86::CPU cpu{};
        cpu.esp = kStackHigh - 0x100;
        if (!g_blocks.empty() && (g_blocks.size() > 24 || chance(50)))
        {
            cpu.eax = takeBlock();
            nfree.generated(&g, cpu);
        }
        else
        {
            cpu.eax = anAmount();
            nmalloc.generated(&g, cpu);
            if (!cpu.terminate && cpu.eax != 0)
                g_blocks.push_back(cpu.eax);
        }
        keepHeapSound(g);
    }
}

/* A struct tm as the time functions are handed one: sensible, or its
 * fields out of their ranges -- months past 11, seconds past 59, negative
 * ones -- or anything. */
void makeTm(CheckApp& g, x86::reg32 tm)
{
    static const x86::reg32 ranges[] = {60, 60, 24, 31, 12, 70, 7, 366};
    for (x86::reg32 i = 0; i < 8; ++i)
    {
        x86::reg32 v = below(ranges[i]) + (i == 3 ? 1 : 0) + (i == 5 ? 70 : 0);
        if (chance(15))
            v = x86::reg32(x86::sreg32(below(2 * ranges[i] * 30)) - x86::sreg32(ranges[i] * 30));
        else if (chance(3))
            v = random32();
        g.word(tm + 4 * i) = v;
    }
    g.word(tm + 0x20) = chance(40) ? 0xffffffff : chance(50) ? 0 : chance(80) ? 1 : random32();
}

/* A daylight saving rule, a struct tm of __tzset's: Mm.w.d (isdst 0) --
 * month, week 1 to 5, weekday -- or Jn (1) or n (2), a day of the year; and
 * a time of day.  Its month near its range, which the runtime indexes by. */
void makeRule(CheckApp& g, x86::reg32 rule)
{
    g.word(rule + 0x20) = chance(60) ? 0 : chance(50) ? 1 : chance(80) ? 2 : random32();
    g.word(rule + 0x10) = chance(95) ? below(12) : below(14) - 1;
    g.word(rule + 0xc) = chance(90) ? 1 + below(5) : below(8);
    g.word(rule + 0x18) = chance(90) ? below(7) : random32();
    g.word(rule + 0x1c) = chance(90) ? below(367) : random32();
    g.word(rule + 8) = chance(90) ? below(24) : below(30);
    g.word(rule + 4) = below(60);
    g.word(rule + 0) = below(60);
}

/* A stream as the runtime has one: open for writing mostly, for reading, or
 * both; buffered fully, by lines, not at all, or none of them yet, or more
 * than one; text or binary, dirty, a tty's, at its end, in error -- and now
 * and then bits no flag has.  Its buffer made or not yet, of a byte or a
 * few, of 0x86, of most of 1K, and part full, or full; its orientation none,
 * bytes or wide characters, and committed on a flush now and then. */
void makeStream(CheckApp& g, x86::reg32 fp, x86::reg32 link, x86::reg32 buffer, x86::reg32 handle)
{
    static const x86::reg32 bufferings[] = {0, 0x100, 0x200, 0x400};
    static const x86::reg32 sizes[] = {1, 2, 3, 0x10, 0x86, 0x200, 0x3f0};
    x86::reg32 flag = chance(75) ? 2 : chance(50) ? 1 : 3;
    flag |= chance(90) ? bufferings[below(4)] : random32() & 0x700;
    for (const x86::reg32 bit : {0x4u, 0x8u, 0x10u, 0x20u, 0x40u, 0x80u, 0x800u, 0x1000u, 0x2000u, 0x4000u})
        if (chance(bit == 0x40 || bit == 0x1000 ? 50 : bit == 0x8 || bit == 0x2000 ? 25 : bit == 0x800 ? 5 : 10))
            flag |= bit;
    if (chance(5))
        flag |= random32() & 0xffff8000;
    const bool made = chance(80);
    const x86::reg32 size = made || chance(30) ? (chance(60) ? sizes[below(7)] : 1 + below(0x3f0)) : 0;
    const x86::reg32 count = !made || size == 0 ? 0 : chance(92) ? below(size) : size;
    for (x86::reg32 at = buffer; at < buffer + 0x400; at += 4)
        g.word(at) = random32();
    for (x86::reg32 at = buffer + below(16); at < buffer + 0x400; at += 1 + below(16))
        g.getMemory<x86::reg8>(at) = x86::reg8("\r\n\x1a"[below(3)]);
    for (x86::reg32 at = fp; at < fp + 0x20; at += 4)
        g.word(at) = random32();
    g.word(fp) = !made ? 0 : buffer + ((flag & 2) ? count : below(size - count + 1));
    g.word(fp + 4) = count;
    g.word(fp + 8) = link;
    g.word(fp + 0xc) = flag;
    g.word(fp + 0x10) = handle;
    g.word(fp + 0x14) = size;
    g.word(link) = 0;
    g.word(link + 4) = fp;
    g.word(link + 8) = made ? buffer : 0;
    g.word(link + 0xc) = chance(60) ? 0 : chance(75) ? 1 : chance(70) ? 2 : random32();
    g.word(link + 0x10) = (chance(20) ? 1 : 0) | (chance(5) ? random32() & ~1u : 0);
}

/* A handle: one of the 20 mostly, the first three often; 20 itself, which
 * the runtime takes, ones past it, negative ones. */
x86::reg32 aHandle()
{
    static const x86::reg32 picks[] = {20, 21, 0xffffffff, 0, 1, 2, 3, 5, 9, 19};
    return chance(40) ? picks[below(10)] : chance(10) ? random32() : below(20);
}

/* A stream: the harness's, or stdout or stderr, stdin now and then. */
x86::reg32 aStream()
{
    return chance(15) ? kIob + 0x1a * (1 + below(2)) : chance(5) ? kIob : kStreams + 0x40 * below(8);
}

/* The heap afresh, empty, VirtualAlloc's slots all free: for the buffers the
 * streams' functions take that nobody gives back. */
void freshHeap(CheckApp& g)
{
    for (const x86::reg32 at : {kHeapFirst, kHeapRover, kHeapLargestBefore, kHeapFreeRover})
        g.word(at) = 0;
    g.word(kSlotsTaken) = 0;
    g_blocks.clear();
}

/* What the case starts from, on `g`: what the functions read of the game. */
void makeState(CheckApp& g)
{
    g_shortDirectory = false;
    // Where an error is from: a file, a function, a line -- or none.
    for (const x86::reg32 where : {kWhere, kCalledFrom})
    {
        g.word(where) = chance(where == kWhere ? 75 : 30) ? aString() : 0;
        g.word(where + 4) = aString();
        g.word(where + 8) = below(2000);
    }
    g.word(kErrorUnderWay) = chance(15) ? 1 + below(3) : 0;
    g.word(kDebugOutput) = chance(25) ? random32() | 1 : 0;
    g.word(kDebugStop) = chance(30) ? 1 : 0;
    g.word(kErrorFilter) = chance(35) ? standInFor(kErrorFilter) : 0;
    g.word(kErrorWhere) = chance(65) ? 1 : 0;
    g.word(kErrorLock) = chance(40) ? standInFor(kErrorLock) : 0;
    g.word(kErrorBefore) = chance(40) ? standInFor(kErrorBefore) : 0;
    g.word(kErrorPrintOk) = chance(50) ? 1 : 0;
    g.word(kErrorOk) = chance(60) ? standInFor(kErrorOk) : 0;
    g.text(kErrorText, below(0x200), random32());
    g.word(kWindow) = chance(85) ? 0x9000 + below(0x100) : 0;
    g.word(kFloatFormat) = standInFor(kFloatFormat);
    g.word(kMbcsOn) = chance(50) ? 1 : 0;

    // iSND: loaded or not, its functions found or not.
    g.word(kSoundModule) = chance(50) ? 0x7700 + below(0x10) * 0x10 : 0;
    g.word(kSoundOptions) = random32();
    static const char* const soundNames[] = {
        "_iSNDdirectcaps@4", "_iSNDdirectstart@8", "_iSNDdirectstop@0", "_iSNDdirectrecordstart@16",
        "_iSNDdirectrecordpacket@4", "_iSNDdirectrecordstop@0", "_iSNDdirectserve@0", "_iSNDdirectvol@8",
        "_iSNDdirectpos3d@12", "_iSNDdirectrate@8", "_iSNDdirectcreate3dbuf@28", "_iSNDdirectremovebuf@4",
        "_iSNDdirectplay3d@28", "_iSNDdirectstopbuf@4",
    };
    for (x86::reg32 i = 0; i < 14; ++i)
        g.word(kSoundFunctions + 4 * i) = chance(97) ? namedStandIn(soundNames[i]) : 0;
    g.word(kSoundLock) = 0x640000 - 0x100 + below(0x10) * 0x10;
    // The game's tick, another, or none yet (SNDcaps sets it; called, the game ends).
    g.word(kSoundTick) = chance(40) ? standInFor(kSoundTick) : chance(60) ? 0x500c80 : 0;

    // THRASH: the renderer's functions, and what its about says.
    g.word(kRendererModule) = random32();
    g.word(kRendererIs3dfx) = below(3);
    for (x86::reg32 at = kThrashSlots; at < kThrashSlotsEnd; at += 4)
        g.word(at) = random32();
    g.word(0x9ef96c) = chance(80) ? namedStandIn("_THRASH_setstate@8") : 0;
    g.word(0x9ef980) = chance(80) ? namedStandIn("_THRASH_about@0") : 0;
    g.word(kAboutRecord) = chance(60) ? 0x33444658 : random32();
    g.word(kAboutRecord + 8) = chance(60) ? 0x68 : chance(50) ? 0x67 : random32();

    // The Watcom runtime's: its thread's data -- rand's seed, where strtok
    // goes on -- and the strings for its string functions.
    makeText(g);
    g.word(kThreadData) = kThread;
    g.word(kThread + 0xc) = random32();
    g.word(kThread + 0x10) = chance(50) ? 0 : copied(g, aText()) + below(8);

    // Its heap, as the cases before left it: its lock the runtime's own -- a
    // bare ret -- or a stand-in; growing, but now and then not; VirtualAlloc's
    // slots its miniheaps', the rest free.
    g.word(kHeapAccess) = chance(50) ? standInFor(kHeapAccess) : kNullAccess;
    g.word(kHeapRelease) = chance(50) ? standInFor(kHeapRelease) : kNullAccess;
    g.word(kHeapEnabled) = chance(97) ? 1 : 0;
    g.word(kHeapBrk) = chance(97) ? 0 : 0xfffffffe;
    g.word(kHeapBlockSize) = 0x10000;
    g.getMemory<x86::reg8>(kHeapClean) = chance(50) ? 1 : 0;
    // Its time zone: west or east, daylight saving's hour or another, its
    // rules -- Mm.w.d, Jn or n -- northern or southern, or no daylight saving
    // at all.
    g.word(kTimezone) = chance(80) ? x86::reg32(3600 * (x86::sreg32(below(25)) - 12)) : random32();
    g.word(kDstAdjust) = chance(80) ? 3600 : below(7201);
    g.word(kDstName) = chance(25) ? kStrings : aString();  // kStrings' first is ""
    for (const x86::reg32 rule : {kDstStart, kDstEnd})
        makeRule(g, rule);

    // Its handles: __OSHandles' HANDLEs, the game's modes -- the first three's
    // isatty asked or not yet -- the file's lock the runtime's own or a
    // stand-in, and now and then the default windowing's hooks, which the
    // game has none of.
    g.word(kOsHandles) = kHandleTable;
    const bool handlesTaken = chance(40);  // none free
    for (x86::reg32 i = 0; i < 32; ++i)
        g.word(kHandleTable + 4 * i) = !handlesTaken && chance(15) ? 0 : random32() | 1;
    g.word(kFileCount) = 20;
    g.word(kIoModes) = kGameIoModes;
    for (x86::reg32 i = 0; i < 20; ++i)
        g.word(kGameIoModes + 4 * i) = random32() & 0xffff;
    g.word(kAccessFile) = chance(50) ? standInFor(kAccessFile) : kNullAccess;
    g.word(kReleaseFile) = chance(50) ? standInFor(kReleaseFile) : kNullAccess;
    const bool windowed = chance(15);
    g.word(kIsWindowedHandle) = windowed ? standInFor(kIsWindowedHandle) : 0;
    g.word(kWindowsStdin) = windowed && chance(70) ? standInFor(kWindowsStdin) : 0;
    g.word(kWindowsStdout) = windowed && chance(70) ? standInFor(kWindowsStdout) : 0;
    for (const x86::reg32 hook : {kRemoveWindowedHandle, kNewWindow, kDestroyWindow})
        g.word(hook) = windowed && chance(70) ? standInFor(hook) : 0;

    // Its streams: the harness's eight and __iob's first three, theirs
    // 0, 1 and 2 mostly; __OpenStreams some of them, in any order;
    // the streams' lock the runtime's own or a stand-in.
    std::vector<x86::reg32> links, closed;
    for (x86::reg32 i = 0; i < 11; ++i)
    {
        const x86::reg32 fp = i < 8 ? kStreams + 0x40 * i : kIob + 0x1a * (i - 8);
        const x86::reg32 link = i < 8 ? fp + 0x20 : kIobLinks + 0x20 * (i - 8);
        // A stream's handle one isatty can take, which looks at none: of
        // the 20, 20 itself, 21 or -1.
        static const x86::reg32 handles[] = {20, 21, 0xffffffff};
        const x86::reg32 handle = chance(95) ? below(20) : handles[below(3)];
        makeStream(g, fp, link, kStreamBuffers + 0x400 * i, i < 8 || chance(10) ? handle : i - 8);
        if (chance(70))
            links.insert(links.begin() + below(x86::reg32(links.size()) + 1), link);
        else if (chance(50))
            closed.insert(closed.begin() + below(x86::reg32(closed.size()) + 1), link);
    }
    if (chance(10))
        links.clear();
    if (chance(30))
        closed.clear();
    for (const auto* list : {&links, &closed})
    {
        g.word(list == &links ? kOpenStreams : kClosedStreams) = list->empty() ? 0 : list->front();
        for (size_t i = 0; i + 1 < list->size(); ++i)
            g.word((*list)[i]) = (*list)[i + 1];
    }
    // The rest of __iob in use or free, all of it now and then.
    const bool iobTaken = chance(30);
    for (x86::reg32 i = 3; i < 20; ++i)
        g.word(kIob + 0x1a * i + 0xc) = iobTaken || chance(50) ? (random32() & ~3u) | (1 + below(3)) : 0;
    g.word(kAccessIob) = chance(50) ? standInFor(kAccessIob) : kNullAccess;
    g.word(kReleaseIob) = chance(50) ? standInFor(kReleaseIob) : kNullAccess;

    // Opening and closing's: __OSHandles 20 long mostly; the handles' lock
    // the runtime's or a stand-in; __allocPOSIXHandle and __freePOSIXHandle
    // stand-ins where the runtime calls them through; _commode, _fmode and
    // the umask as the game has them, or otherwise.
    g.word(kHandleCount) = chance(60) ? 20 : chance(60) ? 19 + below(3) : below(33);
    g.word(kFakeHandle) = 0x80000000 + below(8);
    g.word(kAccessHandles) = chance(50) ? standInFor(kAccessHandles) : kNullAccess;
    g.word(kReleaseHandles) = chance(50) ? standInFor(kReleaseHandles) : kNullAccess;
    g.word(kAllocHandle) = standInFor(kAllocHandle);
    g.word(kFreeHandle) = standInFor(kFreeHandle);
    g.word(kCommitMode) = chance(80) ? 0 : chance(80) ? 1 : random32();
    g.word(kFileMode) = chance(60) ? 0x100 : chance(80) ? 0x200 : random32();
    g.word(kUmask) = chance(70) ? 0 : chance(50) ? 0x80 : random32() & 0x1ff;

    x86::reg32 taken = 0;
    for (const x86::reg32 heap : miniheaps(g))
        taken |= 1u << (heap - kHeap) / kSlotSize;
    g.word(kSlotsTaken) = taken;
}

/* A string to look for in the string at `in`, written into a buffer: a piece
 * of it, a byte of it, nothing, or anything. */
x86::reg32 lookedFor(CheckApp& g, x86::reg32 in)
{
    const x86::reg32 buffer = aBuffer();
    x86::reg32 length = 0;
    while (g.getMemory<x86::reg8>(in + length) != 0)
        ++length;
    x86::reg32 from = in, count = 0;
    switch (below(5))
    {
    case 0: break;
    case 1: count = length ? 1 : 0; from = in + (length ? below(length) : 0); break;
    case 2: from = in + below(length + 1); count = below(length - (from - in) + 1); break;
    case 3: from = textString(below(32)); count = below(6); break;
    default: from = in + below(length + 1); count = below(length - (from - in) + 1); break;
    }
    for (x86::reg32 i = 0; i < count; ++i)
        g.getMemory<x86::reg8>(buffer + i) = g.getMemory<x86::reg8>(from + i);
    g.getMemory<x86::reg8>(buffer + count) = 0;
    if (count != 0 && chance(20))
        g.getMemory<x86::reg8>(buffer + below(count)) ^= 1;
    return buffer;
}

/* qsort's and bsearch's elements: `n` of `size` bytes at `base`, anything
 * but their keys -- the first dword or the first byte, as `function`
 * compares them -- which are from a few values, for ties, mostly the same,
 * or anything; or in its order, for bsearch. */
void makeElements(CheckApp& g, x86::reg32 base, x86::reg32 n, x86::reg32 size, x86::reg32 function, bool sorted)
{
    for (x86::reg32 at = kElements; at < base + n * size + 8; at += 4)
        g.word(at) = random32();
    const x86::reg32 kind = below(3);
    x86::reg32 key = function == kByByte ? below(0x40) : random32() >> (8 + below(20));
    for (x86::reg32 i = 0; i < n; ++i)
    {
        if (sorted)
            key += below(3);
        else
            key = kind == 0 ? below(4) : kind == 1 ? (chance(85) ? 1 : below(3)) : random32();
        if (function == kByByte)
            g.getMemory<x86::reg8>(base + i * size) = x86::reg8(std::min<x86::reg32>(key, 0xff));
        else
            g.word(base + i * size) = key;
    }
}

/* The formatted output's cases (native_watcom.cpp's __prtf and the rest).
 * Their memory, in kBuffers: what sprintf writes from the first on, the
 * places %n writes to in the 12th, the SPECS and __prtf's buffer in the 11th,
 * the format in the 13th, its arguments in the 14th -- the va_list's dword
 * at the end of the 12th -- and wide strings in the 15th. */
constexpr x86::reg32 kFormatOut = kBuffers;
constexpr x86::reg32 kFormatDigits = kBuffers + 0x200 * 10;
constexpr x86::reg32 kFormatSpecs = kBuffers + 0x200 * 11;
constexpr x86::reg32 kFormatTargets = kBuffers + 0x200 * 12;
constexpr x86::reg32 kFormatList = kBuffers + 0x200 * 12 + 0x1fc;
constexpr x86::reg32 kFormatText = kBuffers + 0x200 * 13;
constexpr x86::reg32 kFormatArguments = kBuffers + 0x200 * 14;
constexpr x86::reg32 kWideTexts = kBuffers + 0x200 * 15;

/* Wide strings, 8 of 0x20 characters at most: letters, anything, none. */
void makeWideTexts(CheckApp& g)
{
    for (x86::reg32 i = 0; i < 8; ++i)
    {
        const x86::reg32 at = kWideTexts + 0x40 * i;
        const x86::reg32 length = chance(10) ? 0 : below(0x20);
        for (x86::reg32 j = 0; j < length; ++j)
            g.getMemory<x86::reg16>(at + 2 * j) = x86::reg16(chance(70) ? 'a' + below(26) : 1 + below(0xffff));
        g.getMemory<x86::reg16>(at + 2 * length) = 0;
    }
}

x86::reg32 aWideText()
{
    return kWideTexts + 0x40 * below(8);
}

/* An integer argument of every kind: small, either sign, anything. */
x86::reg32 anInteger()
{
    switch (below(6))
    {
    case 0: return below(10);
    case 1: return below(100000);
    case 2: return x86::reg32(-x86::sreg32(1 + below(100000)));
    case 3: return chance(50) ? 0x80000000 : chance(50) ? 0x7fffffff : 0xffffffff;
    case 4: return below(0x10000) | (chance(50) ? 0x8000 : 0);
    default: return random32();
    }
}

/* The argument words a conversion takes, as __prtf reads them: its
 * character, flags and second flags as the SPECS has them. */
void conversionArguments(x86::reg8 c, x86::reg8 flags, x86::reg8 flags2, std::vector<x86::reg32>& a)
{
    const bool far_ = flags & 0x80, longLong = flags2 & 1;
    switch (c)
    {
    case 'd': case 'i': case 'u': case 'x': case 'X': case 'o':
        a.push_back(anInteger());
        if (longLong)
            a.push_back(chance(50) ? anInteger() : chance(50) ? 0 : 0xffffffff);
        break;
    case 'n':
        a.push_back(kFormatTargets + 4 * below(0x40));
        if (far_)
            a.push_back(random32());
        break;
    case 's': case 'S':
    {
        // Never a null wide string: __prtf reads its own buffer for it, as
        // wide characters on past the byte it wrote, into what is left on
        // the stack -- by the natives under it (the streams') otherwise than
        // by the game's code, which they are held to only from esp up.
        const bool wide = c == 'S' ? !(flags & 0x10) : (flags & 0x20) != 0;
        a.push_back(wide ? aWideText() : chance(8) ? 0 : chance(80) ? aText() : aString());
        if (far_)
            a.push_back(a.back() == 0 && chance(50) ? 0 : chance(70) ? 0 : below(0x100));
        break;
    }
    case 'p': case 'P':
        a.push_back(random32());
        if (far_)
            a.push_back(random32());
        break;
    case 'c': case 'C':
        a.push_back(chance(60) ? 'a' + below(26) : random32());
        break;
    case 'e': case 'E': case 'g': case 'G':
        a.push_back(random32());
        a.push_back(random32());
        break;
    case 'f': case 'F':
        a.push_back(random32());
        if (!(flags & 0x10))
            a.push_back(random32());
        break;
    default:
        break;
    }
}

/* A format of text and conversions of every kind, at `at`, and the words of
 * the arguments it takes, as __prtf walks it: a '*' takes a width or a
 * precision, a conversion its argument by its character and size -- each one
 * that will do for it, a string for %s, a place for %n.  Far %n, which writes
 * where a register left by the generated code points, is turned into %d.
 *
 * With the streams under it (vfprintf's lock and buffer before __prtf), %lc is
 * %hc: for a character WideCharToMultiByte does not take, __prtf writes its
 * buffer's first byte as the stack had it -- left there by the streams'
 * natives, which keep no registers on it, otherwise than by the game's code. */
std::vector<x86::reg32> makeFormat(CheckApp& g, x86::reg32 at, bool streams = false)
{
    static const char* const sizes[] = {"", "", "", "", "h", "l", "w", "N", "F", "L", "I64", "I6"};
    static const char conversions[] = "ddiiuuxXoccsssSpPneEfgGCq%";
    std::string f;
    const unsigned parts = below(6);
    for (unsigned i = 0; i < parts; ++i)
    {
        for (unsigned j = below(chance(70) ? 6 : 24); j-- > 0;)
            f += chance(90) ? char('a' + below(26)) : " .,:-+0123456789"[below(16)];
        f += '%';
        for (unsigned j = chance(50) ? below(4) : 0; j-- > 0;)
            f += "-#+ 0"[below(5)];
        if (chance(15))
            f += '*';
        else if (chance(40))
            f += std::to_string(below(30));
        if (chance(35))
        {
            f += '.';
            if (chance(25))
                f += '*';
            else if (chance(80))
                f += std::to_string(below(20));
        }
        f += sizes[below(12)];
        f += chance(97) ? conversions[below(sizeof conversions - 1)] : char(1 + below(127));
    }
    for (unsigned j = below(8); j-- > 0;)
        f += char('a' + below(26));
    if (chance(5))
        f += '%';

    // Walked as __prtf walks it.
    std::vector<x86::reg32> a;
    for (size_t i = 0; i < f.size();)
    {
        if (f[i++] != '%')
            continue;
        x86::reg8 flags = 0, flags2 = 0;
        while (i < f.size() && std::strchr("-#+ 0", f[i]))
            ++i;
        auto number = [&] {
            const x86::reg32 n = chance(80) ? below(30) : x86::reg32(-x86::sreg32(below(30)));
            a.push_back(n);
        };
        if (i < f.size() && f[i] == '*')
        {
            number();
            ++i;
        }
        else
            while (i < f.size() && f[i] >= '0' && f[i] <= '9')
                ++i;
        if (i < f.size() && f[i] == '.')
        {
            ++i;
            if (i < f.size() && f[i] == '*')
            {
                number();
                ++i;
            }
            else
                while (i < f.size() && f[i] >= '0' && f[i] <= '9')
                    ++i;
        }
        if (i < f.size())
            switch (f[i])
            {
            case 'N': flags |= 0x40; ++i; break;
            case 'l': case 'w': flags |= 0x20; ++i; break;
            case 'h': flags |= 0x10; ++i; break;
            case 'L': flags2 |= 1; ++i; break;
            case 'F': flags |= 0x80; ++i; break;
            case 'I':
                if (f.compare(i, 3, "I64") == 0)
                {
                    flags2 |= 1;
                    i += 3;
                }
                break;
            }
        if (i >= f.size())
            break;
        if (f[i] == 'n' && (flags & 0x80) && !(flags & 0x20))
            f[i] = 'd';
        if (streams && f[i] == 'c' && (flags & 0x20))
        {
            f[i - 1] = 'h';
            flags = 0x10;
        }
        conversionArguments(x86::reg8(f[i++]), flags, flags2, a);
    }
    std::memcpy(&g.getMemory<char>(at), f.c_str(), f.size() + 1);
    return a;
}

/* SPECS as __prtf makes them, made up: where its characters go, a width, a
 * precision or none, flags of every kind, a conversion. */
void makeSpecs(CheckApp& g, x86::reg32 specs)
{
    static const char conversions[] = "diouxXcsSpPeEfFgGCnq%";
    g.word(specs + 0x00) = kFormatOut + below(0x40);
    g.word(specs + 0x04) = chance(80) ? below(30) : random32();
    g.word(specs + 0x08) = chance(40) ? 0xffffffff : chance(90) ? below(20) : x86::reg32(-x86::sreg32(2 + below(8)));
    g.word(specs + 0x10) = below(0x100);
    g.getMemory<x86::reg8>(specs + 0x15) = x86::reg8(conversions[below(sizeof conversions - 1)]);
    g.getMemory<x86::reg8>(specs + 0x16) = chance(50) ? ' ' : '0';
    g.getMemory<x86::reg8>(specs + 0x1e) = x86::reg8(random32());
    g.getMemory<x86::reg8>(specs + 0x1f) = chance(70) ? 0 : x86::reg8(random32());
    for (x86::reg32 at = specs + 0x20; at < specs + 0x38; at += 4)
        g.word(at) = chance(70) ? below(8) : chance(50) ? 0 : x86::reg32(-x86::sreg32(below(4)));
}

/* A stream for vfprintf and file_putc, as fputc's cases have one. */
x86::reg32 aFormattedStream(CheckApp& g)
{
    if (chance(10))
        freshHeap(g);
    if (chance(20))
    {
        g.word(kIsWindowedHandle) = standInFor(kIsWindowedHandle);
        g.word(kWindowsStdout) = standInFor(kWindowsStdout);
    }
    return aStream();
}

/* The formatted output's registers and stack words; false for a function not
 * of it. */
bool makeFormattedArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    static const char* const names[] = {
        "mem_putc", "file_putc", "vsprintf", "sprintf", "vfprintf", "fprintf", "prtf", "prtfSpec",
        "prtfFlags", "prtfLength", "prtfWideLength", "prtfHex", "prtfHexEnd", "prtfFixed", "prtfFloat", "prtfZeros",
        "prtfWideOut", "prtfConvert", "prtfUpper", "wctomb_", "utoa", "itoa", "ulltoa", "prtfUtoa", "U8D"};
    if (std::find_if(std::begin(names), std::end(names), [&](const char* n) { return name == n; }) == std::end(names))
        return false;
    makeWideTexts(g);
    for (x86::reg32 i = 0; i < 0x100; ++i)
        g.getMemory<x86::reg8>(0xa0b211 + i) = chance(20) ? x86::reg8(random32() | 1) : 0;
    auto aBase = [] { static const x86::reg32 bases[] = {10, 10, 16, 8, 2, 36}; return chance(80) ? bases[below(6)] : 2 + below(35); };

    if (name == "prtf" || name == "vsprintf" || name == "vfprintf" || name == "sprintf" || name == "fprintf")
    {
        const bool streams = name == "vfprintf" || name == "fprintf";
        const std::vector<x86::reg32> a = makeFormat(g, kFormatText, streams);
        const bool toFile = streams || (name == "prtf" && chance(15));
        const x86::reg32 to = toFile ? aFormattedStream(g) : kFormatOut + below(0x40);
        if (name == "sprintf" || name == "fprintf")
        {
            // cdecl: the destination and the format, the arguments after.
            stack[0] = to;
            stack[1] = kFormatText;
            for (size_t i = 0; i < a.size(); ++i)
                if (i + 2 < 9)
                    stack[i + 2] = a[i];
                else
                    g.word(cpu.esp + 4 + 4 * x86::reg32(i + 2)) = a[i];
            return true;
        }
        for (size_t i = 0; i < a.size(); ++i)
            g.word(kFormatArguments + 4 * x86::reg32(i)) = a[i];
        g.word(kFormatList) = kFormatArguments;
        cpu.eax = to;
        cpu.edx = kFormatText;
        cpu.ebx = kFormatList;
        cpu.ecx = toFile ? kFilePutc : kMemPutc;
    }
    else if (name == "mem_putc" || name == "file_putc")
    {
        makeSpecs(g, kFormatSpecs);
        if (name == "file_putc")
            g.word(kFormatSpecs) = aFormattedStream(g);
        cpu.eax = kFormatSpecs;
        cpu.edx = chance(20) ? '\n' : chance(80) ? below(256) : random32();
    }
    else if (name == "prtfSpec" || name == "prtfFlags")
    {
        // What follows a '%': the walk's own format, from one of its '%'s.
        makeSpecs(g, kFormatSpecs);
        const std::vector<x86::reg32> a = makeFormat(g, kFormatText);
        for (size_t i = 0; i < a.size(); ++i)
            g.word(kFormatArguments + 4 * x86::reg32(i)) = a[i];
        g.word(kFormatList) = kFormatArguments;
        const char* text = &g.getMemory<char>(kFormatText);
        const char* percent = std::strchr(text, '%');
        cpu.eax = percent ? kFormatText + x86::reg32(percent - text) + 1 : kFormatText;
        if (name == "prtfFlags")
            cpu.edx = kFormatSpecs;
        else
        {
            cpu.edx = kFormatList;
            cpu.ebx = kFormatSpecs;
        }
    }
    else if (name == "prtfLength")
    {
        cpu.eax = chance(80) ? aText() : aBuffer();
        cpu.edx = random32();
        cpu.ebx = chance(50) ? 0xffffffff : below(0x120);
    }
    else if (name == "prtfWideLength")
    {
        cpu.eax = aWideText();
        cpu.edx = random32();
        cpu.ebx = chance(50) ? 0xffffffff : chance(10) ? random32() : below(0x40);
    }
    else if (name == "prtfHex")
    {
        cpu.eax = random32() >> below(32);
        cpu.edx = kFormatDigits + 0x40 + below(0x40);
        cpu.ebx = chance(80) ? (chance(50) ? 8 : 4) : below(14);
    }
    else if (name == "prtfFixed")
    {
        makeSpecs(g, kFormatSpecs);
        cpu.eax = kFormatDigits + below(0x40);
        cpu.edx = chance(30) ? (random32() & 0xffff) | (chance(50) ? 0xffff0000 : 0) : random32();
        cpu.ebx = kFormatSpecs;
        g.word(kFormatSpecs + 8) = chance(30) ? 0xffffffff : below(12);
    }
    else if (name == "prtfFloat" || name == "prtfConvert")
    {
        makeSpecs(g, kFormatSpecs);
        std::vector<x86::reg32> a;
        conversionArguments(g.getMemory<x86::reg8>(kFormatSpecs + 0x15), g.getMemory<x86::reg8>(kFormatSpecs + 0x1e),
                            g.getMemory<x86::reg8>(kFormatSpecs + 0x1f), a);
        if (g.getMemory<x86::reg8>(kFormatSpecs + 0x15) == 'n')
            a.clear();  // the default's: no argument
        for (size_t i = 0; i < a.size(); ++i)
            g.word(kFormatArguments + 4 * x86::reg32(i)) = a[i];
        g.word(kFormatList) = kFormatArguments;
        cpu.eax = kFormatSpecs + 0x38;
        cpu.edx = kFormatList;
        cpu.ebx = kFormatSpecs;
        cpu.ecx = kFormatSpecs + 0x6c;
    }
    else if (name == "prtfZeros")
    {
        makeSpecs(g, kFormatSpecs);
        cpu.eax = kFormatSpecs;
    }
    else if (name == "prtfWideOut")
    {
        makeSpecs(g, kFormatSpecs);
        const bool toFile = chance(15);
        if (toFile)
            g.word(kFormatSpecs) = aFormattedStream(g);
        g.word(kFormatSpecs + 0x28) = chance(80) ? below(0x40) : chance(50) ? 0 : x86::reg32(-x86::sreg32(below(8)));
        cpu.eax = aWideText();
        cpu.edx = random32();
        cpu.ebx = kFormatSpecs;
        cpu.ecx = toFile ? kFilePutc : kMemPutc;
    }
    else if (name == "prtfUpper")
    {
        cpu.eax = copied(g, aText());
    }
    else if (name == "wctomb_")
    {
        cpu.eax = chance(10) ? 0 : kFormatDigits + below(0x40);
        cpu.edx = chance(70) ? 'a' + below(26) : random32();
    }
    else if (name == "utoa" || name == "itoa" || name == "prtfUtoa")
    {
        cpu.eax = anInteger();
        cpu.edx = kFormatDigits + below(0x40);
        cpu.ebx = aBase();
    }
    else if (name == "ulltoa")
    {
        const x86::reg32 value = kFormatDigits + 0x100;
        g.word(value) = anInteger();
        g.word(value + 4) = chance(30) ? 0 : chance(30) ? 0xffffffff : anInteger();
        cpu.eax = value;
        cpu.edx = kFormatDigits + below(0x40);
        cpu.ebx = aBase();
    }
    else if (name == "U8D")
    {
        // A divisor of one word or two -- 1, below the dividend's high word,
        // above it, the same -- never 0.
        cpu.eax = random32();
        cpu.edx = chance(30) ? below(16) : random32();
        switch (below(6))
        {
        case 0: cpu.ecx = 0; cpu.ebx = 1; break;
        case 1: cpu.ecx = 0; cpu.ebx = 1 + below(36); break;
        case 2: cpu.ecx = 0; cpu.ebx = random32() | 1; break;
        case 3: cpu.ecx = cpu.edx; cpu.ebx = chance(50) ? cpu.eax : random32(); break;
        case 4: cpu.ecx = random32() >> below(32); cpu.ebx = random32(); break;
        default: cpu.ecx = 1 + below(8); cpu.ebx = random32(); break;
        }
        if (cpu.ecx == 0 && cpu.ebx == 0)
            cpu.ebx = 1;
    }
    return true;
}

/* The environment's, the time zone's and the way out's cases (native_watcom.cpp,
 * after the formatted output).  Their memory: the environment's strings in the
 * 13th of kBuffers, its pointers in the 14th, strings to work on in the 10th,
 * what they write in the 9th. */
constexpr x86::reg32 kEnvironStrings = kBuffers + 0x200 * 13;
constexpr x86::reg32 kEnvironList = kBuffers + 0x200 * 14;
constexpr x86::reg32 kWorkText = kBuffers + 0x200 * 10;
constexpr x86::reg32 kWorkOut = kBuffers + 0x200 * 9;
constexpr x86::reg32 kEnvironVar = 0xa0b154;
constexpr x86::reg32 kCodePageVar = 0x56b0b8;
constexpr x86::reg32 kFiniTable = 0x56b81a, kFiniTableEnd = 0x56b844;
constexpr x86::reg32 kExitHooks[] = {0x564484, 0x564488, 0x5677f0, 0x5677a0, 0x5677ac};
constexpr x86::reg32 kFinalizer = 0x56b800;  // a finalizer's stand-in, keyed here
constexpr x86::reg32 kExitStartedVar = 0x9f6420, kEnvironCopies = 0x9f6424;
constexpr x86::reg32 kAtexitCountVar = 0x9f3890, kDebuggerVar = 0x56af64, kZoneDoneVar = 0x568ba0;
constexpr x86::reg32 kTmpDirVar = 0x568278;

/* A string of the kind TZ holds, or of one of its parts. */
std::string aZone()
{
    static const char* const zones[] = {
        "EST5EDT", "PST8PDT,M3.2.0/2,M11.1.0", "CET-1CEST,M3.5.0,M10.5.0/3", "JST-9", "GMT0", "UTC",
        ":Europe/Paris", "XYZ+3:30ABC-4:15:20,J60/1:2:3,100", "AAA-12BBB", "NZST-12NZDT,M9.5.0,M4.1.0/3",
        "ABCDEFGHIJ1", "Q", "", "EST5EDT,J1,J365/25", "ZZZ5ZZY,M12.5.6/0:0:1,M1.1.1/23:59:59", "-3", ",,,"};
    if (chance(80))
        return zones[below(17)];
    std::string z;
    for (unsigned i = below(24); i-- > 0;)
        z += "ABCZ0123456789+-:,./JM"[below(22)];
    return z;
}

/* Bytes of every kind for the multibyte functions: letters, lead bytes and
 * what follows them, 0 now and then early. */
void makeMultibyteText(CheckApp& g, x86::reg32 at, x86::reg32 length)
{
    for (x86::reg32 i = 0; i < length; ++i)
        g.getMemory<x86::reg8>(at + i) = x86::reg8(chance(50) ? "aBcDzZ09_="[below(10)]
                                                   : chance(50) ? 0x81 + below(0x1f) : 1 + below(255));
    g.getMemory<x86::reg8>(at + length) = 0;
}

/* The environment: some of TZ, TMP and the like, in any case, and others; or
 * none at all. */
void makeEnvironment(CheckApp& g)
{
    static const char* const names[] = {"TZ", "tz", "Tz", "TMP", "TEMP", "TMPDIR", "TEMPDIR", "tmp", "PATH",
                                        "NFS", "T", "TZZ", "COMSPEC", ""};
    if (chance(5))
    {
        g.word(kEnvironVar) = 0;
        return;
    }
    x86::reg32 text = kEnvironStrings;
    const x86::reg32 count = below(10);
    for (x86::reg32 i = 0; i < count; ++i)
    {
        std::string s = chance(90) ? names[below(14)] : std::string(1, char('A' + below(26)));
        if (chance(95))
            s += '=';
        const std::string& name = s;
        if (name.compare(0, 2, "TZ") == 0 || name.compare(0, 2, "tz") == 0 || name.compare(0, 2, "Tz") == 0)
            s += aZone();
        else
            s += chance(20) ? "" : chance(10) ? std::string(0x110, 'x') : chance(50) ? "C:\\TEMP" : "c:/tmp/";
        if (text + s.size() + 1 > kEnvironStrings + 0x1f0)
            break;
        std::memcpy(&g.getMemory<char>(text), s.c_str(), s.size() + 1);
        g.word(kEnvironList + 4 * i) = text;
        text += x86::reg32(s.size()) + 1;
        g.word(kEnvironList + 4 * (i + 1)) = 0;
    }
    g.word(kEnvironList + 4 * count) = 0;
    if (count == 0)
        g.word(kEnvironList) = 0;
    g.word(kEnvironVar) = kEnvironList;
}

/* The environment's, the time zone's and the way out's registers and stack
 * words; false for a function not of them. */
bool makeEnvironmentArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    static const char* const names[] = {
        "mbterm", "mbsinc", "mbclen", "mbccpy", "mbccmp", "mbsnextc", "mbvtop", "mbbtrail", "mbclegal",
        "mbclower", "mbcupper932", "mbctoupper", "mbsupr", "mbcicmp", "mbsnicmp", "getenv", "wcstombs",
        "tzNumber", "tzName", "tzRule", "tzWindows", "tzDaylight", "tzset", "callRoutine", "FiniRtns",
        "DoneExceptionHandler", "freeEnvironment", "exit_", "_exit", "exit", "atexit", "EnterWVIDEO",
        "fatalWrite", "fatal_runtime_error", "sleep", "getpid", "stackavail", "GRO", "hexDigit", "fullpath",
        "tmpdir", "MkTmpFile"};
    if (std::find_if(std::begin(names), std::end(names), [&](const char* n) { return name == n; }) == std::end(names))
        return false;

    g_shortDirectory = true;
    // What all of them may read: multibyte characters on or off, of code page
    // 932 or another; the environment; the way out's hooks and finalizers,
    // stand-ins all; the thread's data.
    g.word(kMbcsOn) = chance(40) ? 1 : 0;
    g.word(kCodePageVar) = chance(30) ? 932 : 1252;
    for (x86::reg32 i = 0; i < 0x100; ++i)
        g.getMemory<x86::reg8>(0xa0b211 + i) = chance(25) ? x86::reg8(random32() | 1) : 0;
    makeEnvironment(g);
    for (const x86::reg32 hook : kExitHooks)
        g.word(hook) = (hook == 0x5677f0 || hook == 0x5677ac) && chance(40) ? 0 : standInFor(hook);
    for (x86::reg32 at = kFiniTable; at < kFiniTableEnd; at += 6)
    {
        g.getMemory<x86::reg8>(at) = x86::reg8(chance(25) ? 2 : below(2));
        g.getMemory<x86::reg8>(at + 1) = x86::reg8(chance(50) ? below(0x40) : random32());
        g.getMemory<x86::reg32>(at + 2) = chance(20) ? 0 : standInFor(kFinalizer);
    }
    g.word(kExitStartedVar) = chance(50) ? 0 : chance(50) ? 1 : random32();
    static const char* const freeing[] = {"freeEnvironment", "exit_", "_exit", "exit", "fatalWrite",
                                          "fatal_runtime_error"};
    const bool frees = std::find_if(std::begin(freeing), std::end(freeing),
                                    [&](const char* n) { return name == n; }) != std::end(freeing);
    for (const x86::reg32 at : {kEnvironCopies, kEnvironCopies + 4})
        g.word(at) = frees && !g_blocks.empty() && chance(30) ? takeBlock() : 0;
    g.word(kThread + 0x54) = chance(50) ? 0 : kWorkOut + 0x1f0;
    g.word(kWorkOut + 0x1f0) = random32();
    g.word(kThread) = 0xef0000 + below(0x10000);  // the stack's bottom
    g.getMemory<x86::reg8>(kDebuggerVar) = chance(10) ? 1 : 0;
    g.getMemory<x86::reg8>(kZoneDoneVar) = x86::reg8(below(4));
    g.word(kAtexitCountVar) = chance(80) ? below(0x20) : 0x1e + below(4);
    if (chance(50))
        g.getMemory<x86::reg8>(kTmpDirVar) = 0;
    else
        std::strcpy(&g.getMemory<char>(kTmpDirVar), chance(50) ? "C:\\TEMP\\" : "d:/x");
    makeMultibyteText(g, kWorkText, below(0x40));
    makeMultibyteText(g, kWorkText + 0x80, below(0x40));
    if (chance(40))
        std::memcpy(&g.getMemory<x86::reg8>(kWorkText + 0x80), &g.getMemory<x86::reg8>(kWorkText), 0x40);
    for (x86::reg32 i = 0; i < 0x20; ++i)
        g.getMemory<x86::reg16>(kWorkText + 0x100 + 2 * i) =
            x86::reg16(i == 0x1f || chance(8) ? 0 : chance(70) ? 'a' + below(26) : random32());

    auto aCharacter = [] {
        return chance(40) ? 'a' + below(26) : chance(30) ? (0x81 + below(0x1f)) << 8 | below(256)
               : chance(50) ? below(256) : random32() & 0xffff;
    };
    auto aText = [] { return kWorkText + (chance(50) ? 0 : 0x80) + below(4); };
    if (name == "mbterm" || name == "mbsinc" || name == "mbclen" || name == "mbsnextc" || name == "mbsupr")
        cpu.eax = aText();
    else if (name == "mbccpy" || name == "mbccmp" || name == "mbcicmp" || name == "mbsnicmp")
    {
        cpu.eax = name == "mbccpy" ? kWorkOut + below(0x10) : aText();
        cpu.edx = aText();
        cpu.ebx = chance(10) ? 0 : chance(10) ? 0xffffffff : below(0x30);
    }
    else if (name == "mbvtop")
    {
        cpu.eax = aCharacter();
        cpu.edx = kWorkOut + below(0x10);
    }
    else if (name == "mbbtrail" || name == "mbclegal" || name == "mbclower" || name == "mbcupper932"
             || name == "mbctoupper")
        cpu.eax = name == "mbbtrail" ? (chance(70) ? below(256) : random32()) : aCharacter();
    else if (name == "getenv")
    {
        static const char* const looked[] = {"TZ", "tz", "TMP", "TEMP", "PATH", "T", "", "NONE", "TZZ"};
        std::strcpy(&g.getMemory<char>(kWorkOut), looked[below(9)]);
        cpu.eax = chance(5) ? 0 : chance(80) ? kWorkOut : aText();
    }
    else if (name == "wcstombs")
    {
        cpu.eax = chance(20) ? 0 : kWorkOut + below(0x10);
        cpu.edx = kWorkText + 0x100 + 2 * below(0x10);
        cpu.ebx = chance(10) ? 0 : below(0x40);
    }
    else if (name == "tzNumber" || name == "tzName" || name == "tzRule" || name == "tzDaylight")
    {
        const std::string z = name == "tzNumber" ? std::to_string(random32() >> below(32)) + (chance(50) ? "x" : "")
                                                 : aZone();
        std::strcpy(&g.getMemory<char>(kWorkText), z.c_str());
        cpu.eax = kWorkText + (z.empty() ? 0 : below(x86::reg32(z.size())));
        cpu.edx = kWorkOut;
        cpu.ebx = kWorkOut + 0x100;
        cpu.ecx = cpu.eax;
    }
    else if (name == "callRoutine")
    {
        g.word(kWorkOut) = chance(30) ? 0 : standInFor(kFinalizer);
        cpu.eax = kWorkOut;
    }
    else if (name == "FiniRtns")
    {
        cpu.eax = chance(70) ? (chance(50) ? 0 : 0x10) : below(0x100);
        cpu.edx = chance(70) ? (chance(50) ? 0xff : 0xf) : below(0x100);
    }
    else if (name == "exit_" || name == "_exit" || name == "exit" || name == "sleep")
        cpu.eax = chance(50) ? below(4) : random32();
    else if (name == "atexit")
        cpu.eax = random32();
    else if (name == "EnterWVIDEO" || name == "fatalWrite" || name == "fatal_runtime_error")
    {
        cpu.eax = aString();
        cpu.edx = chance(50) ? 1 : random32();
    }
    else if (name == "GRO")
        stack[0] = chance(20) ? 0 : below(0x6000);
    else if (name == "hexDigit")
        cpu.eax = chance(90) ? below(16) : random32();
    else if (name == "fullpath")
    {
        static const char* const paths[] = {"con", "CON", "", "x", "C:\\GAME\\NFS3", "..\\a\\b", "data"};
        std::strcpy(&g.getMemory<char>(kWorkText), paths[below(7)]);
        cpu.eax = chance(25) ? 0 : kWorkOut;
        cpu.edx = chance(10) ? 0 : kWorkText;
        cpu.ebx = chance(20) ? below(5) : chance(80) ? 0x104 : below(0x200);
    }
    else if (name == "MkTmpFile")
    {
        cpu.eax = kWorkOut;
        cpu.edx = random32();
    }
    return true;
}

/* setenv's and the heap's growing's cases (native_watcom.cpp, after the
 * environment's).  _environ, _wenviron and __env_mask are made as the game
 * makes them -- by the native setenv, run on `g` before the case, from no
 * environment at all, on a heap afresh now and then -- so their blocks are
 * the heap's own; the case's arguments are names and values from the 10th of
 * kBuffers, wide ones from its second half. */
constexpr x86::reg32 kWideEnvironVar = 0xa0b158, kEnvMaskVar = 0xa0b150;
constexpr x86::reg32 kSetenvNames = kBuffers + 0x200 * 10;   // 8 of 0x20
constexpr x86::reg32 kSetenvValues = kSetenvNames + 0x100;   // 8 of 0x20
constexpr x86::reg32 kSetenvWide = kBuffers + 0x200 * 15;    // 8 of 0x40: "name=value"
constexpr x86::reg32 kSetenvScratch = kBuffers + 0x200 * 12;  // a wide name or two, what is copied

/* A block of the heap's, left on the list. */
x86::reg32 aBlock()
{
    return g_blocks.empty() ? 0 : g_blocks[below(x86::reg32(g_blocks.size()))];
}

/* A block from the native _nmalloc, on `g`, with the cpu's stack below the
 * case's; 0 for none. */
x86::reg32 blockBefore(CheckApp& g, x86::reg32 esp, x86::reg32 size)
{
    x86::CPU c{};
    c.esp = esp - 0x2000 - 4;
    c.eax = size;
    nfs3hp::watcom::nmalloc(&g, c);
    return c.eax;
}

/* The native setenv, on `g`, with the cpu's stack below the case's. */
void setenvBefore(CheckApp& g, x86::reg32 esp, x86::reg32 name, x86::reg32 value, x86::reg32 overwrite, bool wide)
{
    x86::CPU c{};
    c.esp = esp - 0x2000;
    c.eax = name;
    c.edx = value;
    c.ebx = overwrite;
    c.esp -= 4;
    (wide ? nfs3hp::watcom::wsetenv : nfs3hp::watcom::setenv)(&g, c);
}

/* The setenv family's registers; false for a function not of it. */
bool makeSetenvArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    static const char* const names[] = {
        "mbslen", "mbtowc", "mbstowcs", "wcslen", "memcpy", "wcscpy", "towupper", "memmove", "msize",
        "HeapManager_expand", "nexpand", "nexpandFailed", "nrealloc", "findenv", "setenv_", "wfindenv",
        "wsetenv", "wputenvFind", "wputenv", "create_wide_environment", "setenv"};
    if (std::find_if(std::begin(names), std::end(names), [&](const char* n) { return name == n; }) == std::end(names))
        return false;
    g_shortDirectory = true;
    g.word(kMbcsOn) = chance(30) ? 1 : 0;
    g.word(kCodePageVar) = chance(20) ? 932 : 1252;
    // Lead bytes above ASCII, as code pages have them: getenv's names
    // compared one way and __findenv's another would take out a variable
    // getenv says is not there.
    for (x86::reg32 i = 0; i < 0x100; ++i)
        g.getMemory<x86::reg8>(0xa0b211 + i) = i >= 0x81 && chance(15) ? x86::reg8(random32() | 1) : 0;

    // Names -- some the same but for case -- values, wide "name=value"s.
    static const char* const variables[] = {"TZ", "tz", "TMP", "PATH", "A", "NFS3", "a", "Tz"};
    static const char* const values[] = {"", "x", "EST5EDT", "C:\\TEMP", "1", "\x81\x40z", "long value here", "y"};
    for (x86::reg32 i = 0; i < 8; ++i)
    {
        std::strcpy(&g.getMemory<char>(kSetenvNames + 0x20 * i), variables[i]);
        std::strcpy(&g.getMemory<char>(kSetenvValues + 0x20 * i), values[i]);
        const std::string w = std::string(variables[below(8)]) + (chance(90) ? "=" : "") + values[below(8)];
        for (x86::reg32 j = 0; j <= w.size(); ++j)
            g.getMemory<x86::reg16>(kSetenvWide + 0x40 * i + 2 * j) = j < w.size() ? x86::reg8(w[j]) : 0;
    }
    auto aName = [] { return kSetenvNames + 0x20 * below(8); };
    auto aValue = [] { return chance(10) ? 0 : kSetenvValues + 0x20 * below(8); };

    // The environment, as setenv makes it: none, narrow, or narrow and wide.
    const bool environment = name == "findenv" || name == "setenv_" || name == "wfindenv" || name == "wsetenv"
                             || name == "wputenvFind" || name == "wputenv" || name == "create_wide_environment"
                             || name == "setenv";
    if (environment)
    {
        if (chance(30))
            freshHeap(g);
        g.word(kEnvironVar) = 0;
        g.word(kWideEnvironVar) = 0;
        g.word(kEnvMaskVar) = 0;
        // __findenv and the wide ones are called on an environment that is
        // there only: on none they read address 0 for one.
        const bool wideNeeded = name == "wfindenv" || name == "wputenvFind";
        const bool needed = wideNeeded || name == "findenv";
        const x86::reg32 narrow = !needed && chance(15) ? 0 : 1 + below(6);
        for (x86::reg32 i = 0; i < narrow; ++i)
            setenvBefore(g, cpu.esp, aName(), kSetenvValues + 0x20 * (1 + below(7)), below(2), false);
        if (needed && g.word(kEnvironVar) == 0)
        {
            // setenv failed each time (SetEnvironmentVariableA's stand-in
            // said no): _environ as the start-up makes it, none of it the
            // runtime's.
            do
                makeEnvironment(g);
            while (g.word(kEnvironVar) == 0);
            x86::reg32 count = 0;
            while (g.word(g.word(kEnvironVar) + 4 * count) != 0)
                ++count;
            g.word(kEnvMaskVar) = g.word(kEnvironVar) + 4 * (count + 1);
            for (x86::reg32 i = 0; i < 0x20; i += 4)
                g.word(g.word(kEnvMaskVar) + i) = 0;
        }
        // _wenviron for the wide ones only: made, it takes __env_mask over --
        // the runtime keeps one for both -- and _environ's own go wrong after.
        const bool wide = name == "wsetenv" || name == "wputenv" || name == "create_wide_environment";
        if (wideNeeded || (wide && chance(60)))
        {
            g.getMemory<x86::reg16>(kSetenvScratch) = 'Q';
            g.getMemory<x86::reg16>(kSetenvScratch + 2) = 0;
            g.getMemory<x86::reg16>(kSetenvScratch + 4) = 'v';
            g.getMemory<x86::reg16>(kSetenvScratch + 6) = 0;
            setenvBefore(g, cpu.esp, kSetenvScratch, kSetenvScratch + 4, 1, true);
        }
        if (wideNeeded && g.word(kWideEnvironVar) == 0)
        {
            // Not made (a stand-in said no): _wenviron of the wide strings as
            // they are, __env_mask none of them the runtime's.
            const x86::reg32 list = kSetenvScratch + 0x140, mask = kSetenvScratch + 0x180;
            const x86::reg32 count = 1 + below(7);
            for (x86::reg32 i = 0; i < count; ++i)
                g.word(list + 4 * i) = kSetenvWide + 0x40 * i;
            g.word(list + 4 * count) = 0;
            for (x86::reg32 i = 0; i < 0x20; i += 4)
                g.word(mask + i) = 0;
            g.word(kWideEnvironVar) = list;
            g.word(kEnvMaskVar) = mask;
        }
    }

    if (name == "mbslen")
        cpu.eax = kSetenvValues + 0x20 * below(8);
    else if (name == "mbtowc" || name == "mbstowcs")
    {
        cpu.eax = chance(20) ? 0 : kSetenvScratch + 2 * below(8);
        cpu.edx = name == "mbtowc" && chance(10) ? 0 : kSetenvValues + 0x20 * below(8) + below(2);
        cpu.ebx = chance(10) ? 0 : name == "mbtowc" ? below(4) : below(0x20);
    }
    else if (name == "wcslen" || name == "wcscpy")
    {
        cpu.eax = name == "wcscpy" ? kSetenvScratch + 2 * below(8) : kSetenvWide + 0x40 * below(8);
        cpu.edx = kSetenvWide + 0x40 * below(8);
    }
    else if (name == "memcpy" || name == "memmove")
    {
        // Into another buffer, or over the source either way.
        cpu.edx = kSetenvScratch + 0x40 + below(0x40);
        cpu.eax = name == "memmove" && chance(60) ? cpu.edx + below(0x40) - 0x20 : kSetenvScratch + 0x100 + below(0x40);
        cpu.ebx = chance(10) ? 0 : below(0x80);
    }
    else if (name == "towupper")
        cpu.eax = chance(70) ? 'A' + below(60) : random32();
    else if (name == "msize")
        cpu.eax = aBlock() != 0 && chance(80) ? aBlock() : kSetenvScratch + 8;
    else if (name == "HeapManager_expand" || name == "nexpand" || name == "nrealloc")
    {
        // A block of the heap's, to any size; for _nrealloc none, or 0 bytes.
        if (g_blocks.empty())
            g_blocks.push_back(blockBefore(g, cpu.esp, 1 + below(0x200)));
        // _nexpand's a block of its own, made now: one of the list's may have
        // gone into another that grew over it, free as it was.
        x86::reg32 block = name == "nrealloc" ? (chance(90) ? takeBlock() : 0)
                                              : blockBefore(g, cpu.esp, 1 + below(0x200));
        // None -- the heap full, or VirtualAlloc's stand-in said no -- a
        // block from the heap afresh, as often as it takes.
        while (block == 0 && name != "nrealloc")
        {
            freshHeap(g);
            block = blockBefore(g, cpu.esp, 1 + below(0x200));
        }
        // The heap of segment ds: another's, the code takes esi as it finds it.
        cpu.eax = name == "HeapManager_expand" ? cpu.ds : block;
        cpu.edx = name == "HeapManager_expand" ? block : chance(5) ? 0 : chance(50) ? below(0x40) : below(0x1000);
        cpu.ebx = chance(50) ? below(0x40) : below(0x1000);
        cpu.ecx = kSetenvScratch;
    }
    else if (name == "findenv" || name == "setenv_" || name == "setenv")
    {
        cpu.eax = chance(5) ? 0 : chance(5) ? kSetenvValues : aName();
        cpu.edx = aValue();
        cpu.ebx = below(2);
        // Taking a variable out, __findenv goes on through _environ moved
        // down, __env_mask after it, whose bytes it takes for strings: on
        // the runtime's own, a byte of 1 each, to anywhere.  So for that --
        // no value, or setenv's empty one that may be none -- _environ as the
        // start-up makes it, none of it the runtime's; setenv's empty one
        // without overwriting otherwise, which takes out none that is there.
        if (name == "setenv" && cpu.edx != 0 && g.getMemory<x86::reg8>(cpu.edx) == 0)
            cpu.ebx = 0;
        if (cpu.edx == 0)
        {
            do
                makeEnvironment(g);  // none at all for __findenv: it reads address 0 for one
            while (name == "findenv" && g.word(kEnvironVar) == 0);
            const x86::reg32 env = g.word(kEnvironVar);
            if (env != 0)
            {
                x86::reg32 count = 0;
                while (g.word(env + 4 * count) != 0)
                    ++count;
                g.word(kEnvMaskVar) = env + 4 * (count + 1);
                for (x86::reg32 i = 0; i < 0x20; i += 4)
                    g.word(env + 4 * (count + 1) + i) = 0;
            }
        }
    }
    else if (name == "wfindenv" || name == "wsetenv")
    {
        g.getMemory<x86::reg16>(kSetenvScratch + 0x100) = 'T';
        g.getMemory<x86::reg16>(kSetenvScratch + 0x102) = chance(50) ? 'z' : 0;
        g.getMemory<x86::reg16>(kSetenvScratch + 0x104) = 0;
        cpu.eax = chance(5) ? 0 : chance(70) ? kSetenvScratch + 0x100 : kSetenvScratch;
        cpu.edx = chance(15) ? 0 : kSetenvWide + 0x40 * below(8);
        cpu.ebx = below(2);
    }
    else if (name == "wputenvFind" || name == "wputenv")
    {
        cpu.eax = chance(5) && name == "wputenv" ? 0 : kSetenvWide + 0x40 * below(8);
        cpu.edx = below(2);
    }
    return true;
}

/* The Watcom runtime's functions' registers -- eax, edx, ebx, ecx -- and the
 * x87 for __CHP; `stack` the words on the stack. */
/* Formatted input's cases (native_watcom.cpp, after setenv's): a format of
 * conversions with the arguments it takes, pointers into kScanTargets, and
 * an input made to go with it now and then otherwise -- signs, bases, points
 * and exponents, words, sets, multibyte characters, the input over early.
 * No conversion is 'F': a far pointer's les is not the generated code's, and
 * it would store through whatever register it leaves.  A number the float's
 * conversion takes is short of its buffer on the stack: one past 0x50
 * characters runs over the code's frame. */
constexpr x86::reg32 kScanText = kBuffers + 0x200 * 11;     // the input: 0x100
constexpr x86::reg32 kScanFormat = kScanText + 0x100;       // the format: 0x100
constexpr x86::reg32 kScanTargets = kBuffers + 0x200 * 12;  // what the conversions store: 8 of 0x40
constexpr x86::reg32 kScanArgs = kBuffers + 0x200 * 13;     // the arguments: 16 pointers
constexpr x86::reg32 kScanList = kScanArgs + 0x40;          // a va_list's: the arguments' pointer
constexpr x86::reg32 kScanSpecs = kScanArgs + 0x60;         // __scnf's specs, 0x14 bytes
constexpr x86::reg32 kScanFormatAt = kScanArgs + 0x80;      // scan_arb's: the format's pointer
constexpr x86::reg32 kScanNumber = kScanArgs + 0xa0;        // a long double, 10 bytes, and its room
constexpr x86::reg32 kScanEnd = kScanArgs + 0xc0;           // strtod's end
constexpr x86::reg32 kScanSet = kScanArgs + 0x100;          // makelist's 32 bytes
constexpr x86::reg32 kScanFloatPointer = 0x56ac68;          // __EFG_scanf's pointer

/* A number as strtod and the float's conversion take one: a sign, digits, a
 * point among them, an exponent of either sign; leading white space for
 * strtod; now and then none of it, or hex, or a huge or tiny exponent. */
std::string aNumberText(bool space)
{
    std::string s;
    if (space && chance(30))
        s += chance(50) ? " \t" : " ";
    if (chance(40))
        s += chance(50) ? '-' : '+';
    if (chance(5))
        return s + (chance(50) ? "x1" : "");
    const unsigned whole = chance(20) ? 0 : below(chance(20) ? 25 : 8);
    for (unsigned i = 0; i < whole; ++i)
        s += char(chance(15) ? '0' : '0' + below(10));
    if (chance(60) || whole == 0)
    {
        s += '.';
        const unsigned fraction = below(chance(10) ? 22 : 7);
        for (unsigned i = 0; i < fraction; ++i)
            s += char(chance(15) ? '0' : '0' + below(10));
    }
    if (chance(40))
    {
        s += chance(50) ? 'e' : 'E';
        if (chance(50))
            s += chance(50) ? '-' : '+';
        const unsigned digits = chance(10) ? 0 : 1 + below(chance(20) ? 5 : 3);
        for (unsigned i = 0; i < digits; ++i)
            s += char('0' + below(10));
    }
    if (chance(20))
        s += "x.5";
    return s;
}

/* An integer's text for a base -- 0 for the number's own -- a sign, 0x or
 * 0, digits of the base and past it, now and then a segment's ':'. */
std::string anIntegerText(x86::reg32 base)
{
    static const char digits[] = "0123456789abcdefABCDEFgz";
    std::string s;
    if (chance(30))
        s += chance(50) ? '-' : '+';
    if (chance(25))
        s += chance(60) ? (chance(50) ? "0x" : "0X") : "0";
    const unsigned count = chance(10) ? 0 : 1 + below(chance(20) ? 24 : 10);
    for (unsigned i = 0; i < count; ++i)
        s += digits[below(base == 10 || base == 8 ? (chance(90) ? 10 : 24) : 24)];
    if (chance(10))
        s += ":" + std::to_string(below(100));
    return s;
}

/* A word: letters, now and then bytes above 0x7f -- lead bytes with what
 * follows them. */
std::string aWordText()
{
    std::string s;
    const unsigned count = 1 + below(chance(20) ? 24 : 8);
    for (unsigned i = 0; i < count; ++i)
        s += char(chance(10) ? 0x81 + below(0x7e) : chance(80) ? 'a' + below(26) : '!' + below(15));
    return s;
}

/* A format of conversions, into kScanFormat, with the pointers its
 * assigning ones take, into kScanArgs, and an input into kScanText made to
 * go with it mostly. */
void makeScan(CheckApp& g)
{
    static const char* const kinds[] = {"d", "i", "o", "u", "x", "X", "p", "e", "f", "g", "E", "G", "s", "S", "c",
                                        "C", "[", "n", "%"};
    std::string format, input;
    const unsigned count = 1 + below(6);
    for (unsigned c = 0; c < count; ++c)
    {
        if (chance(20))
            format += chance(50) ? " " : "\t ";
        if (chance(10))
        {
            const char literal = char(chance(50) ? ',' : 'a' + below(26));
            format += literal;
            input += chance(85) ? literal : '?';
        }
        format += '%';
        const std::string kind = kinds[below(19)];
        if (kind == "%")
        {
            format += '%';
            input += chance(85) ? "%" : "q";
            continue;
        }
        if (chance(15))
            format += '*';
        x86::reg32 width = 0;
        if (chance(30))
        {
            width = 1 + below(20);
            format += std::to_string(width);
        }
        if (chance(10))
            format += 'N';
        if (chance(35))
        {
            static const char* const sizes[] = {"h", "l", "w", "L", "I64", "I6"};
            format += sizes[below(6)];
        }
        if (kind == "[")
        {
            std::string set = chance(30) ? "^" : "";
            const unsigned members = below(8);
            for (unsigned i = 0; i < members; ++i)
                set += char(chance(20) ? ']' : 'a' + below(8));
            format += "[" + set + (chance(90) ? "]" : "");
        }
        else
            format += kind;
        // The input for it: white space first now and then, then its text.
        if (chance(40))
            input += chance(50) ? " " : "\n\t ";
        const char k = kind[0];
        if (k == 'd' || k == 'u')
            input += anIntegerText(10);
        else if (k == 'i')
            input += anIntegerText(0);
        else if (k == 'o')
            input += anIntegerText(8);
        else if (k == 'x' || k == 'X' || k == 'p')
            input += anIntegerText(16);
        else if (k == 'e' || k == 'f' || k == 'g' || k == 'E' || k == 'G')
            input += aNumberText(false);
        else if (k == 's' || k == 'S' || k == 'c' || k == 'C')
            input += aWordText();
        else if (k == '[')
        {
            const unsigned length = below(10);
            for (unsigned i = 0; i < length; ++i)
                input += char(chance(70) ? 'a' + below(8) : 'p' + below(8));
        }
        if (chance(30))
            input += ' ';
    }
    if (chance(15) && !input.empty())
        input.resize(below(x86::reg32(input.size())));  // the input over early
    if (chance(10))
        input += aWordText();
    format.resize(std::min<size_t>(format.size(), 0xf0));
    input.resize(std::min<size_t>(input.size(), 0xf0));
    std::strcpy(&g.getMemory<char>(kScanFormat), format.c_str());
    std::strcpy(&g.getMemory<char>(kScanText), input.c_str());
    for (x86::reg32 i = 0; i < 16; ++i)
        g.word(kScanArgs + 4 * i) = kScanTargets + 0x40 * (i < 8 ? i : 7);
    for (x86::reg32 at = kScanTargets; at < kScanTargets + 0x200; at += 4)
        g.word(at) = random32();
    g.word(kScanList) = kScanArgs;
}

/* __scnf's specs at kScanSpecs: sscanf's routines, the input, a width, flags
 * -- any but a far pointer's. */
void makeScanSpecs(CheckApp& g)
{
    g.word(kScanSpecs) = 0x4f04f0;
    g.word(kScanSpecs + 4) = 0x4f051c;
    g.word(kScanSpecs + 8) = kScanText + (chance(10) ? x86::reg32(std::strlen(&g.getMemory<char>(kScanText))) : 0);
    g.word(kScanSpecs + 0xc) = chance(40) ? 0xffffffff : chance(10) ? 0 : 1 + below(20);
    g.getMemory<x86::reg8>(kScanSpecs + 0x10) =
        x86::reg8((chance(85) ? 1 : 0) | (chance(5) ? 2 : 0) | (random32() & 0xf8));
}

/* A long double into `at`: any double's, or any bits at all. */
void aLongDouble(CheckApp& g, x86::reg32 at)
{
    if (chance(80))
        g.getMemory<x86::IEEEf80>(at) = x86::Float(anyDouble());
    else
        for (x86::reg32 i = 0; i < 10; ++i)
            g.getMemory<x86::reg8>(at + i) = x86::reg8(random32());
    if (chance(30))
        g.getMemory<x86::reg16>(at + 8) = x86::reg16(0x3fff + below(40) - 20 + (chance(50) ? 0x8000 : 0));
}

/* The x87 as a case finds it: any control word, a stack of a few. */
void anyX87(x86::CPU& cpu)
{
    static const x86::reg16 precisions[] = {0x000, 0x200, 0x300};
    cpu.fpu.setControl(x86::reg16(0x7f | precisions[below(3)] | below(4) << 10 | (random32() & 0x1000)));
    cpu.fpu.count = below(6);
    for (x86::Float& f : cpu.fpu.regs)
        f = x86::Float(anyDouble());
}

/* The formatted input's registers; false for a function not of it. */
bool makeScanArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    static const char* const names[] = {
        "cget_string", "uncget_string", "vsscanf", "sscanf", "Strtold", "strtod", "cgetc", "uncget", "scnf", "get_opt",
        "scan_white", "scan_char", "scan_string", "report_scan", "makelist", "scan_arb", "scan_float", "scan_int",
        "radix_value", "chktrm", "ZBuf2LD", "normalize", "scalePowers", "scaleEnd", "scaleBy", "LDScale10x", "FDFS",
        "U8M"};
    if (std::find_if(std::begin(names), std::end(names), [&](const char* n) { return name == n; }) == std::end(names))
        return false;
    for (x86::reg32 i = 0; i < 0x100; ++i)
        g.getMemory<x86::reg8>(0xa0b211 + i) = chance(20) ? x86::reg8(random32() | 1) : 0;
    g.word(kScanFloatPointer) = standInFor(kScanFloatPointer);
    makeScan(g);
    makeScanSpecs(g);

    if (name == "sscanf")
    {
        stack[0] = kScanText;
        stack[1] = kScanFormat;
        for (x86::reg32 i = 0; i < 7; ++i)
            stack[2 + i] = g.word(kScanArgs + 4 * i);
        for (x86::reg32 i = 7; i < 12; ++i)
            g.word(cpu.esp + 4 + 4 * (i + 2)) = g.word(kScanArgs + 4 * i);
    }
    else if (name == "vsscanf")
    {
        cpu.eax = kScanText;
        cpu.edx = kScanFormat;
        cpu.ebx = kScanList;
    }
    else if (name == "scnf")
    {
        g.getMemory<x86::reg8>(kScanSpecs + 0x10) &= 0xfb;
        cpu.eax = kScanSpecs;
        cpu.edx = kScanFormat;
        cpu.ebx = kScanList;
    }
    else if (name == "cget_string" || name == "cgetc" || name == "chktrm" || name == "scan_white")
        cpu.eax = kScanSpecs;
    else if (name == "uncget_string" || name == "uncget")
    {
        cpu.eax = random32();
        cpu.edx = kScanSpecs;
    }
    else if (name == "get_opt")
    {
        const char* text = &g.getMemory<char>(kScanFormat);
        const char* percent = std::strchr(text, '%');
        cpu.eax = percent ? kScanFormat + x86::reg32(percent - text) + 1 : kScanFormat;
        cpu.edx = kScanSpecs;
    }
    else if (name == "scan_char" || name == "scan_string" || name == "scan_float" || name == "scan_int"
             || name == "report_scan" || name == "scan_arb")
    {
        cpu.eax = kScanSpecs;
        cpu.edx = kScanList;
        if (name == "scan_int")
        {
            static const x86::reg32 bases[] = {0, 8, 10, 16};
            cpu.ebx = chance(90) ? bases[below(4)] : 2 + below(15);
            cpu.ecx = below(2);
        }
        else if (name == "report_scan")
            cpu.ebx = random32();
        else if (name == "scan_arb")
        {
            static const char* const sets[] = {"abc]", "^abc]", "]a]", "^]x]", "", "a", "^", "dcba"};
            std::strcpy(&g.getMemory<char>(kScanFormat + 0x80), sets[below(8)]);
            g.word(kScanFormatAt) = kScanFormat + 0x80;
            cpu.ebx = kScanFormatAt;
        }
        else if (name == "scan_float")
        {
            const std::string number = aNumberText(false);
            std::strcpy(&g.getMemory<char>(kScanText + 0x80), number.c_str());
            if (chance(70))
                g.word(kScanSpecs + 8) = kScanText + 0x80;
        }
        else if (name == "scan_int")
        {
            const std::string number = anIntegerText(cpu.ebx);
            std::strcpy(&g.getMemory<char>(kScanText + 0x80), number.c_str());
            if (chance(70))
                g.word(kScanSpecs + 8) = kScanText + 0x80;
        }
    }
    else if (name == "makelist")
    {
        cpu.eax = kScanFormat + below(0x10);
        cpu.edx = kScanSet;
    }
    else if (name == "radix_value")
        cpu.eax = chance(80) ? '0' + below(0x50) : random32();
    else if (name == "FDFS")
    {
        const double value = chance(50) ? anyDouble() : std::ldexp(double(random32()), int(below(300)) - 150);
        std::uint64_t bits;
        std::memcpy(&bits, &value, 8);
        cpu.eax = x86::reg32(bits);
        cpu.edx = x86::reg32(bits >> 32);
    }
    else if (name == "U8M")
    {
        cpu.eax = random32();
        cpu.edx = chance(50) ? 0 : random32();
        cpu.ebx = random32();
        cpu.ecx = chance(50) ? 0 : random32();
    }
    else if (name == "normalize")
    {
        cpu.eax = chance(30) ? 0 : random32() >> below(32);
        cpu.edx = chance(30) ? 0 : random32() >> below(32);
        cpu.ebp = chance(30) ? 0 : chance(20) ? 0xffffffff : random32();
        cpu.edi = chance(70) ? 0x405e : random32();
    }
    else if (name == "ZBuf2LD")
    {
        std::string digits;
        const unsigned count = below(chance(10) ? 30 : 20);
        for (unsigned i = 0; i < count; ++i)
            digits += char('0' + below(10));
        std::strcpy(&g.getMemory<char>(kScanText + 0x80), digits.c_str());
        cpu.eax = kScanText + 0x80;
        cpu.edx = kScanNumber;
    }
    else if (name == "scalePowers" || name == "scaleBy" || name == "LDScale10x")
    {
        anyX87(cpu);
        aLongDouble(g, kScanNumber);
        cpu.eax = kScanNumber;
        cpu.edx = chance(60) ? x86::reg32(x86::sreg32(below(80)) - 40)
                             : chance(50) ? x86::reg32(x86::sreg32(below(0x6000)) - 0x3000) : random32();
        if (name == "scalePowers")
        {
            aLongDouble(g, kScanNumber + 0x10);
            cpu.eax = kScanNumber + 0x10;
        }
    }
    else if (name == "scaleEnd")
        cpu.ebp = cpu.esp - 4;  // the frame it leaves: its registers below the return address
    else if (name == "Strtold" || name == "strtod")
    {
        anyX87(cpu);
        std::string number = aNumberText(true);
        if (chance(10))
            number = chance(50) ? "1e400" : "-1e-400";
        std::strcpy(&g.getMemory<char>(kScanText + 0x80), number.c_str());
        cpu.eax = kScanText + 0x80;
        cpu.edx = name == "Strtold" ? kScanNumber : chance(20) ? 0 : kScanEnd;
        cpu.ebx = chance(20) ? 0 : kScanEnd;
    }
    return true;
}

/* The console's cases (native_watcom.cpp, after the formatted input's): its
 * HANDLEs open or not, a key ungot or not, do_getch's state -- a character
 * repeating, an extended key's second half -- and the default windowing's
 * hooks there or not; the console's records made up by the stand-ins. */
constexpr x86::reg32 kConsoleIn = 0x568a38, kConsoleOut = 0x568a3c;
constexpr x86::reg32 kUngotKey = 0x567828, kKeyState = 0x567168;
constexpr x86::reg32 kKeyScan = 0x9f38b0, kKeyChar = 0x9f38b4, kKeyRepeats = 0x9f38b8;
constexpr x86::reg32 kWindowsKbhit = 0x5677e0, kWindowsGetch = 0x5677e4, kWindowsPutch = 0x5677ec;
constexpr x86::reg32 kKeyRecord = kBuffers + 0x200 * 14;  // an INPUT_RECORD

/* The console's registers; false for a function not of it. */
bool makeConsoleArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    static const char* const names[] = {"NTRealKey", "openConsoles", "NTConsoleInput", "NTConsoleOutput",
                                        "putch", "kbhit", "do_getch", "getch", "getche"};
    if (std::find_if(std::begin(names), std::end(names), [&](const char* n) { return name == n; }) == std::end(names))
        return false;
    g.word(kConsoleIn) = chance(50) ? 0xffffffff : 0x7000 + 4 * below(0x100);
    g.word(kConsoleOut) = chance(50) ? 0xffffffff : 0x7000 + 4 * below(0x100);
    g.word(kUngotKey) = chance(20) ? (chance(80) ? 1 + below(0xff) : random32()) : 0;
    g.word(kKeyState) = chance(70) ? below(3) : random32();
    g.word(kKeyScan) = random32() & (chance(80) ? 0xff : 0xffffffff);
    g.word(kKeyChar) = chance(30) ? 0 : 'a' + below(26);
    g.word(kKeyRepeats) = chance(50) ? below(3) : random32();
    for (const x86::reg32 hook : {kWindowsKbhit, kWindowsGetch, kWindowsPutch})
        g.word(hook) = chance(25) ? standInFor(hook) : 0;
    g.word(kIsWindowedHandle) = standInFor(kIsWindowedHandle);
    g.word(kAccessFile) = standInFor(kAccessFile);
    g.word(kReleaseFile) = standInFor(kReleaseFile);
    if (name == "NTRealKey")
    {
        g.getMemory<x86::reg16>(kKeyRecord) = x86::reg16(chance(80) ? 1 : below(0x20));
        g.word(kKeyRecord + 4) = chance(30) ? 0 : chance(80) ? 1 : random32();
        g.getMemory<x86::reg16>(kKeyRecord + 0xa) = x86::reg16(chance(40) ? 0xe + below(7) : random32());
        cpu.eax = kKeyRecord;
    }
    else if (name == "putch")
        cpu.eax = chance(80) ? below(0x100) : random32();
    else if (name == "do_getch")
        cpu.eax = 0x7000 + 4 * below(0x100);
    return true;
}

/* Structured exception handling's cases (native_watcom.cpp, after the
 * console's): a registration and its scopes' table -- each scope in an outer
 * one or none, a filter's or a finally block's, the stand-ins (kFilterCode,
 * kFinallyCode) -- at a level, unwinding to another. */
constexpr x86::reg32 kRegistration = kBuffers + 0x200 * 15;  // 0x1c bytes
constexpr x86::reg32 kScopes = kRegistration + 0x40;         // 6 bytes each
constexpr x86::reg32 kExceptionRecord = kRegistration + 0x100;
constexpr x86::reg32 kChainHead = kRegistration + 0x180;     // fs:[0]

/* The SEH's registers; false for a function not of it. */
bool makeSehArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    static const char* const names[] = {"TryInit", "TryHandler", "TryScope", "TryOuter", "TryDone", "Except",
                                        "global_unwind", "local_unwind", "RtlUnwindThunk"};
    if (std::find_if(std::begin(names), std::end(names), [&](const char* n) { return name == n; }) == std::end(names))
        return false;
    const x86::reg32 scopes = 1 + below(6);
    for (x86::reg32 i = 0; i < scopes; ++i)
    {
        const bool finally = chance(50);
        g.getMemory<x86::reg8>(kScopes + 6 * i) = x86::reg8(i == 0 || chance(30) ? 0xff : below(i));
        g.getMemory<x86::reg8>(kScopes + 6 * i + 1) = x86::reg8(finally ? 1 + below(0xff) : 0);
        g.word(kScopes + 6 * i + 2) = standInFor(finally ? kFinallyCode : kFilterCode);
    }
    auto aLevel = [&] { return chance(15) ? 0xff : below(scopes); };
    g.word(kRegistration) = random32();
    g.word(kRegistration + 4) = 0x4f7b2a;
    g.word(kRegistration + 8) = random32();
    g.word(kRegistration + 0xc) = kScopes;
    g.getMemory<x86::reg8>(kRegistration + 0x10) = x86::reg8(aLevel());
    g.getMemory<x86::reg8>(kRegistration + 0x11) = x86::reg8(random32());
    g.getMemory<x86::reg8>(kRegistration + 0x12) = x86::reg8(aLevel());
    g.word(kExceptionRecord + 4) = chance(40) ? (chance(50) ? 2 : 4) : chance(10) ? random32() : 0;
    g.word(kChainHead) = random32();
    cpu.efs = kChainHead;
    if (name == "TryInit")
        cpu.eax = kRegistration;
    else if (name == "TryHandler")
    {
        stack[0] = kExceptionRecord;
        stack[1] = kRegistration;
    }
    else if (name == "TryScope" || name == "TryOuter")
    {
        cpu.ebp = kRegistration;
        cpu.edi = kScopes;
        cpu.ebx = name == "TryScope" ? aLevel() : 3 * below(scopes);
    }
    else if (name == "Except")
    {
        static const x86::reg32 says[] = {0xffffffff, 0, 1};
        cpu.eax = chance(90) ? says[below(3)] : random32();
        stack[0] = 3 * below(scopes);
        stack[1] = kScopes;
        stack[2] = kRegistration;
    }
    else if (name == "global_unwind" || name == "local_unwind")
        cpu.ebp = kRegistration;
    return true;
}

/* The start's cases (native_watcom.cpp, after structured exception
 * handling's): the initializers' table made up as the finalizers' is, the
 * thread's data a buffer or none -- calloc's then -- the command lines,
 * quoted or not, Windows' version of either kind; the game's main and exit
 * stand-ins. */
constexpr x86::reg32 kInitTable = 0x56b7de, kInitTableEnd = 0x56b81a;
constexpr x86::reg32 kThreadDataSizeVar = 0x56af70, kStackSizeVar = 0x567820;
constexpr x86::reg32 kCommandLineText = kBuffers + 0x200 * 14;          // 0x100
constexpr x86::reg32 kWideCommandLineText = kCommandLineText + 0x100;   // 0x80 characters
constexpr x86::reg32 kStartData = kBuffers + 0x200 * 13;                // a thread's data
constexpr x86::reg32 kStartChain = kBuffers + 0x200 * 15 + 0x180;       // fs:[0]

/* The start's registers; false for a function not of it. */
bool makeStartArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    static const char* const names[] = {"startHook", "InitRtns", "stackLimits", "calloc", "initThread",
                                        "initThreadData", "initPOSIXHandles", "moduleWideName", "copyCommandLine",
                                        "copyWideCommandLine", "InitExceptionHandling", "NTInit", "NTMain",
                                        "callMain", "mainStart", "cstart_"};
    if (std::find_if(std::begin(names), std::end(names), [&](const char* n) { return name == n; }) == std::end(names))
        return false;
    if (chance(10))
        freshHeap(g);
    for (x86::reg32 at = kInitTable; at < kInitTableEnd; at += 6)
    {
        g.getMemory<x86::reg8>(at) = x86::reg8(chance(25) ? 2 : below(2));
        g.getMemory<x86::reg8>(at + 1) = x86::reg8(chance(50) ? below(0x40) : random32());
        g.getMemory<x86::reg32>(at + 2) = chance(20) ? 0 : standInFor(kFinalizer);
    }
    // The thread's data's size: the runtime writes up to its 0xf4th byte.
    g.word(kThreadDataSizeVar) = chance(80) ? 0xf4 : 0xf4 + 4 * below(16);
    g.word(kStackSizeVar) = chance(30) ? 0 : chance(70) ? 0x1000 * below(8) + below(0x10) : below(0x9000);
    g.getMemory<x86::reg16>(0x567841) = x86::reg16(random32() | (chance(50) ? 0x8000 : 0));
    g.getMemory<x86::reg8>(0x56783f) = x86::reg8(3 + below(3));
    // The command lines: the program's name quoted or not, the quote not
    // closed now and then, the arguments after white space.
    static const char* const lines[] = {"\"C:\\Games\\NFS3\\nfs3.exe\"  -window x", "nfs3.exe -nosound",
                                        "\"nfs3.exe", "\"\"", "", "  nfs3", "\"a b\"c d", "nfs3.exe\t\t"};
    const char* line = lines[below(8)];
    std::strcpy(&g.getMemory<char>(kCommandLineText), line);
    for (x86::reg32 i = 0; i <= std::strlen(line); ++i)
        g.getMemory<x86::reg16>(kWideCommandLineText + 2 * i) = x86::reg16(line[i]);
    g.word(kStartChain) = random32();
    cpu.efs = kStartChain;
    for (x86::reg32 at = kStartData; at < kStartData + 0x100; at += 4)
        g.word(at) = random32();
    if (name == "InitRtns")
        cpu.eax = chance(70) ? (chance(50) ? 1 : chance(50) ? 0x21 : 0xff) : below(0x100);
    else if (name == "stackLimits")
    {
        cpu.eax = chance(20) ? 0 : kStartData;
        cpu.edx = chance(20) ? 0 : kStartData + 4;
    }
    else if (name == "calloc")
    {
        cpu.eax = chance(10) ? 0 : 1 + below(8);
        cpu.edx = chance(10) ? 0 : 1 + below(0x40);
    }
    else if (name == "initThread" || name == "initThreadData")
        cpu.eax = chance(30) ? 0 : kStartData;
    else if (name == "moduleWideName")
    {
        cpu.eax = random32();
        cpu.edx = kWorkOut;
        cpu.ebx = chance(70) ? 0x208 : 2 * (1 + below(0x40));
    }
    else if (name == "copyCommandLine")
        cpu.eax = kCommandLineText;
    else if (name == "copyWideCommandLine")
        cpu.eax = kWideCommandLineText;
    else if (name == "InitExceptionHandling")
        cpu.eax = kStartData + 0x100;
    else if (name == "NTInit")
    {
        cpu.eax = chance(70) ? 0 : 1;
        cpu.edx = chance(50) ? 0 : kStartData;
        cpu.ebx = random32();
    }
    else if (name == "NTMain")
    {
        cpu.eax = kStartData + 0x100;
        cpu.edx = chance(50) ? 0 : kStartData;
    }
    return true;
}

/* The maths' error report's and the FDIV bug's cases (native_watcom.cpp,
 * after the start's): long doubles of every kind on the stack and the x87's
 * -- divisors whose mantissas the bug's table names, denormals, zeros,
 * infinities -- the error's flags of every kind, _matherr's say made up. */
constexpr x86::reg32 kMathStruct = kBuffers + 0x200 * 13;  // a struct exception, and doubles after it
constexpr x86::reg32 kMatherrSlot = 0x56b16c;

/* A long double's 10 bytes into the stack's words from `word` on: any
 * double's, or a divisor's mantissa the FDIV bug's table has, or a denormal,
 * or a zero. */
void aLongOnStack(x86::reg32* stack, unsigned word)
{
    x86::reg32 low = random32(), high = random32() | 0x80000000;
    x86::reg32 exponent = 0x3fff + below(64) - 32 + (chance(50) ? 0x8000 : 0);
    switch (below(6))
    {
    case 0:
    {
        // A mantissa of the bug's: the nibble after the integer bit 1, 4, 7,
        // 0xa or 0xd, the next six bits ones.
        static const x86::reg32 nibbles[] = {1, 4, 7, 0xa, 0xd};
        high = 0x80000000 | nibbles[below(5)] << 27 | 0x07e00000 | (random32() & 0x1fffff);
        break;
    }
    case 1: high &= 0x7fffffff; exponent &= 0x8000; break;  // a denormal
    case 2: high = 0; low = chance(50) ? 0 : low; exponent &= 0x8000; break;
    case 3: exponent |= chance(50) ? 0x7fff : 1; break;
    default: break;
    }
    stack[word] = low;
    stack[word + 1] = high;
    stack[word + 2] = (stack[word + 2] & 0xffff0000) | exponent;
}

/* The maths' registers; false for a function not of it. */
bool makeMathArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    static const char* const names[] = {"fdivSafe", "fdiv_fpr", "atanSafe", "fpatan_wrap", "get_std_stream",
                                        "printMathError", "callMatherr", "reportError", "reporterror"};
    if (std::find_if(std::begin(names), std::end(names), [&](const char* n) { return name == n; }) == std::end(names))
        return false;
    static const x86::reg16 precisions[] = {0x000, 0x200, 0x300};
    cpu.fpu.setControl(x86::reg16(0x7f | precisions[below(3)] | below(4) << 10 | (random32() & 0x1000)));
    cpu.fpu.count = 2 + below(2);  // no deeper: see the x87 of the natives' other cases below
    for (x86::Float& f : cpu.fpu.regs)
        f = chance(70) ? x86::Float(double(x86::sreg32(random32() >> below(32))) / double(1 + below(1000)))
                       : x86::Float(anyDouble());
    g.word(kMatherrSlot) = standInFor(kMatherrSlot);
    // The struct exception: a type, a function's name, two arguments, a
    // result.
    g.word(kMathStruct) = chance(90) ? 1 + below(6) : random32() % 8;
    g.word(kMathStruct + 4) = aString();
    for (x86::reg32 i = 8; i < 0x40; i += 8)
    {
        const double value = anyDouble();
        std::memcpy(&g.getMemory<double>(kMathStruct + i), &value, 8);
    }
    if (name == "fdivSafe")
    {
        aLongOnStack(stack, 0);
        aLongOnStack(stack, 3);
    }
    else if (name == "fdiv_fpr")
        cpu.eax = chance(95) ? below(64) : 64 + below(8);
    else if (name == "get_std_stream")
        cpu.eax = chance(80) ? below(5) : random32();
    else if (name == "printMathError")
    {
        cpu.eax = below(7);
        cpu.edx = aString();
    }
    else if (name == "callMatherr")
        cpu.eax = kMathStruct;
    else if (name == "reportError" || name == "reporterror")
    {
        // The function, then of what kind: DOMAIN, SING, OVERFLOW,
        // UNDERFLOW, PLOSS, TLOSS; the result: -HUGE_VAL, 0, 1, HUGE_VAL.
        // Always of a kind: with none the code leaves esi as it was, and
        // prints the type's name from wherever esi points the table.
        static const x86::reg32 kinds[] = {0x40, 0x80, 0x100, 0x800, 0x200, 0x400};
        static const x86::reg32 results[] = {0x1000, 0x2000, 0x4000, 0x8000, 0};
        cpu.eax = (random32() & 0xffff0000) | below(0x16) | kinds[below(6)] | results[below(5)];
        if (chance(10))
            cpu.eax |= random32() & 0xff3f;
        cpu.edx = kMathStruct + 8 * (1 + below(6));
        cpu.ebx = kMathStruct + 8 * (1 + below(6));
    }
    return true;
}

/* The game's fill's entries: a buffer aligned any way, a count of nothing,
 * of a few bytes, of many, below 0 now and then; a pattern of one byte or
 * two, 0, 0xff, or any; the x87 with room on its stack, for the doubles. */
bool makeFillArguments(CheckApp& g, const std::string& name, x86::CPU& cpu)
{
    if (name != "fillZero" && name != "fillByte" && name != "fillWord")
        return false;
    cpu.eax = kBuffers + below(0x40);
    cpu.edx = chance(10) ? below(8) : chance(5) ? 0 - (1 + below(0x40)) : below(0x600);
    for (x86::reg32 at = kBuffers; at < kBuffers + 0x700; at += 4)
        g.word(at) = random32();
    const x86::reg8 b = x86::reg8(chance(20) ? 0 : chance(15) ? 0xff : random32());
    const x86::reg32 spread = b | b << 8 | b << 16 | x86::reg32(b) << 24;
    cpu.ebx = chance(40) ? spread : chance(30) ? (random32() & 0xffff0000) | b << 8 | b : random32();
    static const x86::reg16 precisions[] = {0x000, 0x200, 0x300};
    cpu.fpu.setControl(x86::reg16(0x7f | precisions[below(3)] | below(4) << 10 | (random32() & 0x1000)));
    cpu.fpu.count = below(8);
    for (x86::Float& f : cpu.fpu.regs)
        f = x86::Float(anyDouble());
    return true;
}

void makeWatcomArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    if (makeFormattedArguments(g, name, cpu, stack) || makeEnvironmentArguments(g, name, cpu, stack)
        || makeSetenvArguments(g, name, cpu, stack) || makeScanArguments(g, name, cpu, stack)
        || makeConsoleArguments(g, name, cpu, stack) || makeSehArguments(g, name, cpu, stack)
        || makeStartArguments(g, name, cpu, stack) || makeMathArguments(g, name, cpu, stack))
        return;
    const x86::reg32 i = below(32);
    static const char* const x87[] = {"CHP", "IF_DATAN2", "floor", "ceil", "modf", "control87", "IF_DEXP", "exp",
                                      "IF_DACOS", "DSQRT", "math87_err"};
    if (std::find_if(std::begin(x87), std::end(x87), [&](const char* n) { return name == n; }) != std::end(x87))
    {
        // The x87: a stack of two to four, doubles of every kind on it.  Deeper,
        // what these push themselves would overflow an x87: the natives' pushes
        // wrap over the bottom of the emulated stack, the generated code's stay
        // in its locals, and no game comes here so.
        static const x86::reg16 precisions[] = {0x000, 0x200, 0x300};
        cpu.fpu.setControl(x86::reg16(0x7f | precisions[below(3)] | below(4) << 10 | (random32() & 0x1000)));
        cpu.fpu.count = name == "CHP" ? 1 + below(8) : 2 + below(3);
        for (x86::Float& f : cpu.fpu.regs)
            f = x86::Float(anyDouble());
    }
    if (name == "IF_DACOS")
    {
        // Mostly in [-1, 1], 1 and -1 and 0 themselves; past them; anything.
        static const double edges[] = {1.0, -1.0, 0.0, -0.0, 1.0000001, -0.9999999};
        cpu.fpu.st(0) = x86::Float(chance(60) ? double(x86::sreg32(random32() % 2000001) - 1000000) / 1e6
                                   : chance(50) ? edges[below(6)] : anyDouble());
    }
    else if (name == "IF_DEXP" || name == "DSQRT")
    {
        // For exp either side of its limits, +-709.78; for sqrt below 0.
        if (name == "IF_DEXP" && chance(50))
            cpu.fpu.st(0) = x86::Float(double(x86::sreg32(random32() % 1600001) - 800000) / 1000.0);
        cpu.eax = chance(50) ? (random32() & 0xffffff00) | (chance(50) ? 3 : 1) : random32();
    }
    else if (name == "control87")
    {
        cpu.eax = random32();
        cpu.edx = chance(20) ? 0 : chance(30) ? 0xffff : random32() >> below(32);
        g.getMemory<x86::reg8>(0x5644c0) = chance(80) ? 3 : 0;  // _8087
    }
    else if (name == "floor" || name == "ceil" || name == "exp" || name == "modf" || name == "math87_err")
    {
        // A double on the stack -- for exp either side of its limits -- and
        // modf's pointer, or __math87_err's function.
        double value = anyDouble();
        if (name == "exp" && chance(50))
            value = double(x86::sreg32(random32() % 1600001) - 800000) / 1000.0;
        std::uint64_t bits;
        std::memcpy(&bits, &value, 8);
        stack[0] = x86::reg32(bits);
        stack[1] = x86::reg32(bits >> 32);
        if (name == "modf")
            stack[2] = aBuffer();
        else if (name == "math87_err")
            stack[2] = chance(80) ? below(8) : random32();
    }
    else if (name == "memset")
    {
        cpu.eax = aBuffer() + below(0x40);
        cpu.edx = random32();
        cpu.ebx = chance(10) ? 0 : below(chance(80) ? 0x40 : 0x1c0);
    }
    else if (name == "STOSB" || name == "STOSD")
    {
        cpu.eax = aBuffer() + below(0x40);
        cpu.edx = random32();
        cpu.ecx = chance(10) ? 0 : below(name == "STOSB" ? 0x1c0 : 0x70);
    }
    else if (name == "strcmp" || name == "stricmp" || name == "strncmp")
    {
        cpu.eax = textString(i);
        cpu.edx = chance(10) ? cpu.eax : chance(70) ? textString(i ^ 1) : aText();
        if (chance(50))
            std::swap(cpu.eax, cpu.edx);
        cpu.ebx = chance(10) ? 0 : chance(10) ? 0xffffffff : below(0x20);
    }
    else if (name == "strncpy" || name == "strncat")
    {
        cpu.eax = name == "strncat" ? copied(g, aText()) : aBuffer();
        cpu.edx = aText();
        cpu.ebx = chance(10) ? 0 : chance(10) ? 0xffffffff : below(0x100);
        if (name == "strncpy")
            cpu.ebx = std::min<x86::reg32>(cpu.ebx, 0x1f0);
    }
    else if (name == "strrchr")
    {
        cpu.eax = aText();
        const x86::reg32 length = x86::reg32(std::strlen(&g.getMemory<char>(cpu.eax)));
        cpu.edx = (random32() & 0xffffff00) | (chance(80) ? g.getMemory<x86::reg8>(cpu.eax + below(length + 1)) : below(256));
    }
    else if (name == "strstr")
    {
        cpu.eax = aText();
        cpu.edx = lookedFor(g, cpu.eax);
    }
    else if (name == "setbits" || name == "strtok")
    {
        static const char* const parts[] = {" ", " ,;", "\t\n ", "a", "", "0123456789", "Aa"};
        const x86::reg32 at = aBuffer() + 0x1c0;
        std::strcpy(&g.getMemory<char>(at), chance(70) ? parts[below(7)] : &g.getMemory<char>(textString(i) + below(4)));
        cpu.edx = at;
        cpu.eax = name == "setbits" ? aBuffer() + below(0x40) : chance(30) ? 0 : copied(g, aText());
    }
    else if (name == "strupr")
    {
        cpu.eax = copied(g, aText());
    }
    else if (name == "toupper" || name == "tolower")
    {
        cpu.eax = chance(70) ? below(256) : random32();
    }
    else if (name == "atoi" || name == "atol")
    {
        static const char* const numbers[] = {"0", "123", "-45", "+67", "  89x", "\t\n-2147483648", "4294967297",
                                              "99999999999", " - 5", "--3", "\xff" "5", "12\xff" "3", "", "+", "007"};
        const x86::reg32 at = aBuffer();
        std::strcpy(&g.getMemory<char>(at), chance(60) ? numbers[below(15)] : &g.getMemory<char>(aText()));
        cpu.eax = at;
    }
    else if (name == "srand")
    {
        cpu.eax = random32();
    }
    else if (name == "qsort" || name == "bsearch" || name == "med3")
    {
        // An array: of dwords mostly, aligned mostly; short, middling or long
        // -- either side of 16, 30 and 43 -- compared by its keys, or at random.
        static const x86::reg32 sizes[] = {4, 4, 4, 8, 12, 16, 1, 2, 3, 5, 6, 7, 20, 0};
        const x86::reg32 size = sizes[below(14)];
        const x86::reg32 base = kElements + (chance(25) ? 1 + below(3) : 0);
        static const x86::reg32 from[] = {0, 2, 2, 16, 30, 43, 43, 43, 43, 243};
        static const x86::reg32 more[] = {2, 14, 14, 14, 13, 200, 200, 200, 200, 1500};
        const x86::reg32 pick = below(10);
        x86::reg32 n = from[pick] + below(more[pick]);
        n = std::min<x86::reg32>(n, size == 0 ? 15 : 0xcff0 / size);
        const x86::reg32 function = chance(10) ? kAtRandom : size >= 4 && chance(75) ? kByDword : kByByte;
        makeElements(g, base, n, size, function, name == "bsearch" && chance(80));
        cpu.ecx = standInFor(function);
        if (name == "qsort")
        {
            cpu.eax = base;
            cpu.edx = n;
            cpu.ebx = size;
        }
        else if (name == "med3")
        {
            cpu.eax = base + below(n + 1) * size;
            cpu.edx = base + below(n + 1) * size;
            cpu.ebx = base + below(n + 1) * size;
        }
        else
        {
            // The key an element's, or one near them.
            const x86::reg32 element = base + below(n + 1) * size;
            g.word(kKey) = chance(60) ? g.word(element) : g.word(element) + below(3) - 1;
            cpu.eax = kKey;
            cpu.edx = base;
            cpu.ebx = n;
            cpu.ecx = size;
            stack[0] = standInFor(function);
        }
    }
    else if (name == "set_errno" || name == "set_doserrno")
    {
        cpu.eax = chance(70) ? below(20) : random32();
    }
    else if (name == "set_errno_dos")
    {
        cpu.eax = anError(random32());
    }
    else if (name == "unlink" || name == "remove" || name == "mkdir" || name == "rmdir" || name == "chdir"
             || name == "chmod" || name == "access" || name == "fixed_GetFileAttributesA")
    {
        // A path -- kStrings' have a * or a ? in them now and then -- and a mode.
        cpu.eax = aString();
        cpu.edx = name == "access" ? below(8) : random32();
    }
    else if (name == "getcwd")
    {
        // A buffer, short or long enough, or none.
        cpu.eax = chance(30) ? 0 : aBuffer();
        cpu.edx = chance(80) ? below(0x140) : random32() >> below(32);
        if (cpu.eax != 0)
            cpu.edx = std::min<x86::reg32>(cpu.edx, 0x1ff);
    }
    else if (name == "getdiskfree")
    {
        cpu.eax = chance(30) ? 0 : chance(80) ? 1 + below(26) : random32();
        cpu.edx = aBuffer();
    }
    else if (name == "lseek" || name == "tell" || name == "qread" || name == "qwrite" || name == "GetIOMode"
             || name == "fsync")
    {
        cpu.eax = aHandle();
        if (name == "lseek")
        {
            cpu.edx = chance(30) ? 0 : chance(50) ? below(0x1000) : random32();
            cpu.ebx = chance(80) ? below(3) : random32();
        }
        else if (name == "qread" || name == "qwrite")
        {
            cpu.edx = aBuffer();
            cpu.ebx = chance(10) ? 0 : below(0x200);
        }
    }
    else if (name == "fputc" || name == "fputs" || name == "fwrite" || name == "fflush" || name == "flush"
             || name == "ioalloc" || name == "chktty" || name == "flushall_" || name == "flushall")
    {
        // A stream -- for __ioalloc its buffer not made yet, mostly -- and
        // the heap now and then afresh; the default windowing now and then,
        // for __qwrite's nothing written that is no ENOSPC of its own.
        if (chance(10))
            freshHeap(g);
        if (chance(20))
        {
            g.word(kIsWindowedHandle) = standInFor(kIsWindowedHandle);
            g.word(kWindowsStdout) = standInFor(kWindowsStdout);
        }
        const x86::reg32 fp = aStream();
        if (name == "ioalloc" && chance(80))
        {
            g.word(g.word(fp + 8) + 8) = 0;
            if (chance(70))
                g.word(fp + 0x14) = 0;
        }
        cpu.eax = fp;
        if (name == "fputc")
        {
            // A character: a newline often, with bits above it now and then.
            cpu.eax = chance(20) ? '\n' : chance(5) ? (random32() & ~0xffu) | '\n' : chance(80) ? below(256) : random32();
            cpu.edx = fp;
        }
        else if (name == "fputs")
        {
            cpu.eax = aText();
            cpu.edx = fp;
        }
        else if (name == "fwrite")
        {
            // Up to 2K, a byte at a time or more; none, by n or by size; or
            // n times size 0 as it wraps.
            cpu.eax = chance(50) ? aText() : aBuffer();
            cpu.edx = chance(40) ? 1 : 1 + below(16);
            cpu.ebx = chance(30) ? 1 + below(8) : below(0x80);
            if (chance(5))
                cpu.edx = 0;
            else if (chance(5))
            {
                const bool half = chance(50);
                cpu.edx = half ? 0x80000000 : 4;
                cpu.ebx = half ? 2 : 0x40000000;
            }
            cpu.ecx = fp;
        }
        else if (name == "fflush")
        {
            cpu.eax = chance(25) ? 0 : fp;
        }
        else if (name == "flushall_")
        {
            static const x86::reg32 masks[] = {1, 2, 3, 0x40, 0x1000, 0x2000, 0xffffffff, 0};
            cpu.eax = chance(70) ? masks[below(8)] : random32();
        }
    }
    else if (name == "fgetc" || name == "filbuf" || name == "fill_buffer" || name == "fread" || name == "fgets"
             || name == "fseek" || name == "ftell" || name == "update_buffer" || name == "reset_buffer")
    {
        // A stream open for reading mostly -- stdin often -- and the heap
        // now and then afresh.
        if (chance(10))
            freshHeap(g);
        const x86::reg32 fp = chance(15) ? kIob : aStream();
        if (chance(name == "fseek" || name == "ftell" ? 50 : 80))
            g.word(fp + 0xc) = (g.word(fp + 0xc) | 1) & (chance(50) ? ~2u : ~0u);
        cpu.eax = fp;
        if (name == "fread")
        {
            // Up to 2K, into a buffer with room for it, as for fwrite.
            cpu.eax = kBuffers + 0x200 * below(12);
            cpu.edx = chance(40) ? 1 : 1 + below(16);
            cpu.ebx = chance(30) ? 1 + below(8) : below(0x80);
            if (chance(5))
                cpu.edx = 0;
            else if (chance(5))
            {
                const bool half = chance(50);
                cpu.edx = half ? 0x80000000 : 4;
                cpu.ebx = half ? 2 : 0x40000000;
            }
            cpu.ecx = fp;
        }
        else if (name == "fgets")
        {
            // Room for a few bytes or a line, for none, or less than none.
            cpu.eax = kBuffers + 0x200 * below(15);
            cpu.edx = chance(10) ? below(2) : chance(5) ? 0xfffffffe + below(2) : 1 + below(0x200);
            cpu.ebx = fp;
            // The dword of its frame fgetc's result goes into: for none, -1
            // as often as not.
            g.word(cpu.esp - 0x14) = chance(50) ? 0xffffffff : random32();
        }
        else if (name == "fseek")
        {
            // Near where the buffer has it, from any whence, or anywhere.
            cpu.edx = chance(40) ? x86::reg32(x86::sreg32(below(0x800)) - 0x400)
                      : chance(30) ? below(0x800) : chance(50) ? 0 : random32();
            cpu.ebx = chance(90) ? below(3) : chance(50) ? 3 + below(5) : random32();
        }
        else if (name == "update_buffer")
        {
            cpu.eax = chance(80) ? x86::reg32(x86::sreg32(below(0x800)) - 0x400) : random32();
            cpu.edx = fp;
        }
    }
    else if (name == "fopen" || name == "fsopen" || name == "open_flags" || name == "doopen" || name == "sopen"
             || name == "sopen_" || name == "allocfp" || name == "freefp" || name == "doclose"
             || name == "shutdown_stream" || name == "fclose" || name == "close" || name == "handlesFull"
             || name == "allocPOSIXHandle" || name == "freePOSIXHandle" || name == "NTGetFakeHandle"
             || name == "GetNTAccessAttr" || name == "GetNTShareAttr")
    {
        // A path -- "con" now and then -- a mode string, open's flags and
        // sharing, a stream, a handle; the heap now and then afresh.
        if (chance(10))
            freshHeap(g);
        static const char* const modes[] = {"r", "w", "a", "r+", "w+", "a+", "rb", "wb", "ab", "rt", "r+b", "wb+",
                                            "rc", "wn", "a+c", "rbt", "r++b", "a++c", "wcn", "rbx", "x", "", "R", "wtc"};
        static const char* const consoles[] = {"con", "CON", "Con", "con:"};
        const x86::reg32 path = chance(85) ? aString() : aBuffer() + 0x1e0;
        if (path >= kBuffers)
            std::strcpy(&g.getMemory<char>(path), consoles[below(4)]);
        const x86::reg32 mode = aBuffer() + 0x1f0;
        std::strcpy(&g.getMemory<char>(mode), modes[below(24)]);
        static const x86::reg32 sharings[] = {0, 0x10, 0x20, 0x30, 0x40, 0x50};
        const x86::reg32 shflag = chance(90) ? sharings[below(6)] : random32();
        x86::reg32 oflag = below(4) | (chance(30) ? 0x10 : 0) | (chance(50) ? 0x20 : 0) | (chance(30) ? 0x40 : 0)
                           | (chance(20) ? 0x80 : 0) | (chance(40) ? 0x100 : 0) | (chance(40) ? 0x200 : 0)
                           | (chance(20) ? 0x400 : 0);
        if (chance(5))
            oflag = random32();
        const x86::reg32 permission = chance(70) ? 0x180 : chance(50) ? 0x100 : random32();
        const x86::reg32 fp = aStream();
        if (name == "fopen" || name == "fsopen" || name == "open_flags")
        {
            // _fsopen's sharing one of the five: for another __GetNTShareAttr
            // leaves CreateFileA's as the frame had it, which only the game's
            // code before it in the stack says -- and the game opens with 0.
            cpu.eax = name == "open_flags" ? mode : path;
            cpu.edx = name == "open_flags" ? (chance(90) ? aBuffer() : 0) : mode;
            cpu.ebx = sharings[below(5)];
        }
        else if (name == "doopen")
        {
            static const char firsts[] = "rwaRWAx";
            cpu.eax = path;
            cpu.edx = (random32() & 0xffffff00) | x86::reg8(firsts[below(7)]);
            cpu.ebx = chance(90) ? below(4) | (chance(30) ? 0x40 : 0) | (chance(30) ? 0x80 : 0) : random32();
            cpu.ecx = chance(70) ? below(2) : random32();
            stack[0] = shflag;
            stack[1] = fp;
        }
        else if (name == "sopen")
        {
            stack[0] = path;
            stack[1] = oflag;
            stack[2] = shflag;
            stack[3] = permission;
        }
        else if (name == "sopen_")
        {
            // The va_list: a dword of a buffer, pointing at the permissions.
            const x86::reg32 args = aBuffer() + 0x1c0;
            g.word(args) = args + 4;
            g.word(args + 4) = permission;
            cpu.eax = path;
            cpu.edx = oflag;
            cpu.ebx = shflag;
            cpu.ecx = args;
        }
        else if (name == "freefp" || name == "fclose" || name == "doclose" || name == "shutdown_stream")
        {
            // fclose and __freefp look for the stream: one not open now and then.
            cpu.eax = (name == "fclose" || name == "freefp") && chance(10) ? aBuffer() : fp;
            cpu.edx = chance(70) ? 1 : chance(50) ? 0 : random32();
        }
        else if (name == "close")
        {
            cpu.eax = aHandle();
        }
        else if (name == "allocPOSIXHandle")
        {
            cpu.eax = random32();
        }
        else if (name == "freePOSIXHandle")
        {
            cpu.eax = chance(80) ? below(34) : chance(50) ? 0xffffffff : random32();
        }
        else if (name == "GetNTAccessAttr" || name == "GetNTShareAttr")
        {
            cpu.eax = name == "GetNTAccessAttr" ? (chance(80) ? below(4) : random32())
                                                : (chance(80) ? below(8) << 4 | below(4) : random32());
            cpu.edx = aBuffer();
            cpu.ebx = cpu.edx + 4;
        }
        else
        {
            cpu.eax = random32();
        }
    }
    else if (name == "SetIOMode" || name == "isatty")
    {
        // Neither looks at the handle: one of the 20, or 20 itself.
        cpu.eax = below(21);
        cpu.edx = chance(20) ? 0 : random32();
    }
    else if (name == "leapyear")
    {
        cpu.eax = chance(80) ? 1600 + below(1000) : random32();
    }
    else if (name == "brktime")
    {
        // Days since 1900, 1970's mostly; a time_t; a zone either side of the
        // seconds' turning point; a struct tm.
        cpu.eax = chance(70) ? 0x63df : below(0x10000);
        cpu.edx = chance(30) ? below(0x15180) : random32();
        cpu.ebx = chance(70) ? x86::reg32(3600 * (x86::sreg32(below(25)) - 12)) : random32();
        cpu.ecx = aBuffer();
    }
    else if (name == "time_less")
    {
        // Two times of day, often the same in part.
        cpu.eax = aBuffer();
        cpu.edx = cpu.eax + 0x40;
        for (const x86::reg32 tm : {cpu.eax, cpu.edx})
            for (x86::reg32 i = 0; i < 3; ++i)
                g.word(tm + 4 * i) = chance(10) ? random32() : below(3);
    }
    else if (name == "calc_yday" || name == "check_order")
    {
        cpu.eax = aBuffer();
        cpu.edx = cpu.eax + 0x40;
        makeRule(g, cpu.eax);
        makeRule(g, cpu.edx);
        const x86::reg32 year = chance(90) ? 70 + below(70) : below(200);
        if (name == "calc_yday")
            cpu.edx = year;
        else
            cpu.ebx = year;
    }
    else if (name == "isindst")
    {
        // A struct tm as mktime and _localtime leave one: in range, and
        // its month near it, which the runtime indexes by.
        cpu.eax = aBuffer();
        makeTm(g, cpu.eax);
        g.word(cpu.eax + 0x10) = chance(95) ? below(12) : below(14) - 1;
        g.word(cpu.eax + 0x14) = chance(90) ? 70 + below(70) : below(200);
        // Now and then on the day of the year a rule's Jn or n says, or
        // either side of it, in a leap year as often as not.
        if (chance(30))
        {
            g.word(cpu.eax + 0x1c) = g.word((chance(50) ? kDstStart : kDstEnd) + 0x1c) + below(3) - 1;
            if (chance(50))
                g.word(cpu.eax + 0x14) = 72 + 4 * below(17);
        }
    }
    else if (name == "mktime" || name == "getctime")
    {
        // For mktime now and then a day either side of 1970's first, where a
        // zone west still has a time_t.
        cpu.eax = aBuffer();
        makeTm(g, cpu.eax);
        if (name == "mktime" && chance(15))
        {
            const bool before = chance(50);
            g.word(cpu.eax + 0x14) = before ? 69 : 70;
            g.word(cpu.eax + 0x10) = before ? 11 : 0;
            g.word(cpu.eax + 0xc) = before ? 31 : 1;
            g.word(cpu.eax + 8) = below(24);
        }
    }
    else if (name == "time")
    {
        cpu.eax = chance(30) ? 0 : aBuffer();
    }
    else if (name == "localtime" || name == "localtime_")
    {
        // A time_t, and for _localtime a struct tm.
        cpu.eax = aBuffer();
        g.word(cpu.eax) = chance(70) ? 0x30000000 + below(0x20000000) : random32();
        cpu.edx = cpu.eax + 0x40;
    }
    else if (name == "byteswap")
    {
        // Two places -- apart, the same, or overlapping -- and a count.
        for (x86::reg32 at = kElements; at < kElements + 0x300; at += 4)
            g.word(at) = random32();
        cpu.esi = kElements + below(0x100);
        cpu.edi = chance(20) ? cpu.esi : kElements + below(0x200);
        cpu.ecx = chance(10) ? 0 : below(0x80);
    }
    else if (name == "nmalloc" || name == "CreateNewNHeap" || name == "ExpandDGROUP" || name == "nmemneed")
    {
        cpu.eax = anAmount();
    }
    else if (name == "nfree")
    {
        // A block it gave, none, or memory no miniheap has.
        cpu.eax = !g_blocks.empty() && chance(85) ? takeBlock() : chance(50) ? 0 : aBuffer() + below(0x200);
    }
    else if (name == "MemAllocator")
    {
        // An amount from a miniheap -- none where there is none; edx the segment, anything.
        const std::vector<x86::reg32> heaps = miniheaps(g);
        cpu.edx = random32();
        if (!heaps.empty())
        {
            cpu.ebx = heaps[below(x86::reg32(heaps.size()))];
            cpu.eax = anAmount();
        }
        else
        {
            cpu.eax = 0;
        }
    }
    else if (name == "MemFree")
    {
        // A block it gave and its miniheap, or none.
        cpu.edx = random32();
        cpu.eax = !g_blocks.empty() && chance(90) ? takeBlock() : 0;
        if (cpu.eax != 0)
        {
            cpu.ebx = heapHolding(g, cpu.eax);
            if (cpu.ebx == 0)
                cpu.eax = 0;
        }
    }
    else if (name == "AdjustAmount")
    {
        cpu.eax = aBuffer() + 4 * below(0x10);
        g.word(cpu.eax) = chance(70) ? anAmount() : random32();
        // _amblksiz other than 64K: anything, a few bytes, odd past a page, all but 4 GB.
        if (chance(40))
        {
            const x86::reg32 sizes[] = {random32() >> below(32), below(0x40), 0x1000 * below(0x100) + 1,
                                        0xffffffff - below(0x2000)};
            g.word(kHeapBlockSize) = sizes[below(4)];
        }
    }
    else if (name == "LinkUpNewNHeap")
    {
        // A slot no miniheap has, or memory of the harness's if none is free,
        // its length as __CreateNewNHeap writes it.
        const x86::reg32 taken = g.word(kSlotsTaken);
        x86::reg32 heap = kSamples;
        if (taken != 0xff)
        {
            x86::reg32 slot = below(8);
            while (taken >> slot & 1)
                slot = (slot + 1) % 8;
            heap = kHeap + kSlotSize * slot;
        }
        g.word(heap) = 0x1000 * (1 + below(0x14)) - 4;
        cpu.eax = heap;
    }
    else if (name == "ReturnMemToSystem" || name == "ReleaseNHeap")
    {
        const std::vector<x86::reg32> heaps = miniheaps(g);
        cpu.eax = heaps.empty() ? 0 : heaps[below(x86::reg32(heaps.size()))];
    }
}

/* `from` to `to` made up, a dword at a time: now and then, words at either
 * end of their range, for the saturations. */
void makeSamples(CheckApp& g, x86::reg32 from, x86::reg32 to)
{
    const int loud = chance(50) ? 0 : 25;
    for (x86::reg32 at = from & ~3u; at < to; at += 4)
    {
        g.word(at) = random32();
        if (loud && chance(loud))
            g.getMemory<x86::reg16>(at + 2 * below(2)) = x86::reg16(chance(50) ? 0x7fff - below(2) : 0x8000 + below(2));
    }
}

/* An MMX register's 64 bits: their lanes at their ends now and then. */
x86::reg64 anMmxValue()
{
    static const x86::reg64 edges[] = {0, ~0ull, 0x7fff7fff7fff7fffull, 0x8000800080008000ull, 0x0000ffff0000ffffull,
                                       0x8080808080808080ull};
    return chance(20) ? edges[below(6)] : random32() | x86::reg64(random32()) << 32;
}

/* A count about a block's size: none, a few, whole blocks, one either side,
 * any; a few below 0 now and then, which the x86's signed loops take as
 * their code says. */
x86::reg32 aCount(x86::reg32 block, x86::reg32 most)
{
    if (chance(15))
        return below(2 * block + 2);
    if (chance(6))
        return 0 - (1 + below(2 * block));
    if (chance(30))
        return block * (1 + below(most / block)) + below(3) - 1;
    return 1 + below(most);
}

/* The MMX functions' arguments -- cdecl but for the memmove's copy, in eax,
 * edx and ebx -- their buffers made up, and the MMX registers, which they read
 * before they set some. */
/* The menus' text's cases (native_frontend_text.cpp): numbers of every kind
 * -- the language's lines, the display modes', the cars', the controllers',
 * the ones between that are none -- the language's table in kBuffers, the
 * controllers there are, named or not; names of every length. */
/* A text of the menus' at `at`: up to `most` characters, the font's mostly,
 * now and then one below the space or past its 0xe0. */
void aMenuText(CheckApp& g, x86::reg32 at, x86::reg32 most)
{
    const x86::reg32 length = chance(10) ? 0 : below(most + 1);
    for (x86::reg32 i = 0; i < length; ++i)
        g.getMemory<x86::reg8>(at + i) = x86::reg8(chance(95) ? 0x20 + below(0x5f) : 1 + below(0xff));
    g.getMemory<x86::reg8>(at + length) = 0;
}

/* The world the menus' font and screens see: the two fonts' metrics, their
 * textures' sizes, their lists of quads; the screen, the pointer's bounds,
 * the mouse; the in-race flag, the language and its checks of a line. */
void makeMenusWorld(CheckApp& g, x86::CPU& cpu)
{
    static const x86::reg32 sizes[] = {64, 128, 256, 32, 0};
    for (x86::reg32 at = 0x83f4e0; at < 0x840000; ++at)
        g.getMemory<x86::reg8>(at) = x86::reg8(chance(10) ? random32() : below(0x20));
    for (x86::reg32 font = 0; font < 2; ++font)
    {
        g.getMemory<x86::reg16>(0x83f6a8 + 44 * font) = x86::reg16(sizes[chance(97) ? below(4) : 4]);
        g.getMemory<x86::reg16>(0x83f6aa + 44 * font) = x86::reg16(sizes[chance(97) ? below(4) : 4]);
        const x86::reg32 list = 0x7f74c0 + 0x24010 * font;
        g.word(list + 4) = 4 + below(0x38);
        for (x86::reg32 at = list + 0x10; at < list + 0x800; at += 4)
            g.word(at) = random32();
    }
    g.word(0x840184) = below(2);
    static const float scales[] = {1.0f, 0.5f, 1.25f, 1.6f, 0.8f, 2.0f};
    const float scale = chance(90) ? scales[below(6)] : float(below(4000)) / 1000.0f;
    std::memcpy(&g.word(0x84018c), &scale, 4);
    g.word(0x561b34) = random32();
    g.word(0x561b3c) = chance(50) ? 0 : chance(80) ? 1 : random32();
    static const x86::reg32 widths[] = {640, 800, 1024, 512, 1280};
    g.word(0x7cdac8) = widths[below(5)];
    g.word(0x7cdacc) = g.word(0x7cdac8) * 3 / 4;
    for (x86::reg32 i = 0; i < 4; ++i)
        g.word(0x7cda70 + 4 * i) = chance(80) ? (i < 2 ? below(64) : 320 + below(1000)) : random32();
    g.word(0x559234) = chance(50) ? 0 : chance(80) ? 1 : random32();
    g.word(0x664650) = random32();
    // The pointer: its bounds, where it is; the mouse device and its methods.
    g.word(0x5f38e0) = below(100);
    g.word(0x5f38e4) = below(100);
    g.word(0x5f38e8) = 400 + below(700);
    g.word(0x5f38ec) = 300 + below(500);
    g.word(0x5f38f0) = random32();
    g.word(0x5f38f4) = random32();
    g.word(0x554ee8) = chance(20) ? 0 : kMouseDevice;
    g.word(kMouseDevice) = kMouseDevice + 0x10;
    g.word(kMouseDevice + 0x10 + 0x28) = standInFor(kGetDeviceData);
    // The language, and its checks: none, or the stand-in.
    g.word(0x6fbb4c) = below(8);
    for (x86::reg32 at = 0x558c80; at < 0x558c80 + 8 * 0x14; at += 4)
        g.word(at) = chance(40) ? 0 : standInFor(kLanguageCheck);
    // The menus' art: their buffers, held or not; the detail's glow.
    g.word(0x5f3fdc) = chance(50) ? 0 : kBlocks + 0x400 * below(8);
    g.word(0x5f3fd0) = chance(50) ? 0 : kBlocks + 0x400 * below(8);
    g.getMemory<x86::reg8>(0x7a3a58) = x86::reg8(random32());
    for (x86::reg32 at = kBlocks; at < kBlocks + 0x2000; at += 4)
        g.word(at) = random32();
    // The x87: a control word of every precision and rounding, one or none on its stack.
    static const x86::reg16 precisions[] = {0x000, 0x200, 0x300};
    cpu.fpu.setControl(x86::reg16(0x7f | precisions[below(3)] | below(4) << 10 | (random32() & 0x1000)));
    cpu.fpu.count = below(2);
    for (x86::Float& f : cpu.fpu.regs)
        f = x86::Float(anyDouble());
}

/* A float for the font: a place on the screen mostly, any bits now and then. */
x86::reg32 aPlace()
{
    if (chance(10))
        return anyFloatBits((std::uint64_t(random32()) << 32) | random32());
    const float place = float(x86::sreg32(below(1400)) - 200) + float(below(1000)) / 1000.0f;
    x86::reg32 bits;
    std::memcpy(&bits, &place, 4);
    return bits;
}

bool makeMenusArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    static const char* const names[] = {
        "shortNameCharacter", "shortNameValid", "languageNameValid", "languageFileNameValid", "lineAllowed",
        "textWidth", "selectFont", "measureText", "textWidthWithOverhang", "drawText", "drawCharacter",
        "takeQuad", "viewport", "toScreenX", "toScreenY", "placePointer", "setPointer", "loadScreenArt"};
    if (std::find_if(std::begin(names), std::end(names), [&](const char* n) { return name == n; })
        == std::end(names))
        return false;
    makeMenusWorld(g, cpu);
    const x86::reg32 text = kStrings + 0x40 * 62;
    aMenuText(g, text, 30);
    if (name == "shortNameCharacter")
        cpu.eax = chance(50) ? (cpu.eax & 0xffffff00) | x86::reg8(chance(20) ? '_' : chance(20) ? '-' : random32())
                             : random32();
    else if (name == "shortNameValid" || name == "languageNameValid" || name == "languageFileNameValid"
             || name == "lineAllowed")
    {
        // Names of every length, of letters, digits, '_', '-' and others.
        static const char kinds[] = "abcXYZ019_-. !~";
        const x86::reg32 length = chance(10) ? 0 : 1 + below(12);
        for (x86::reg32 i = 0; i < length; ++i)
            g.getMemory<x86::reg8>(text + i) = x86::reg8(chance(90) ? kinds[below(15)] : 1 + below(0xff));
        g.getMemory<x86::reg8>(text + length) = 0;
        cpu.eax = chance(8) ? 0 : text;
        cpu.edx = chance(85) ? below(3) : random32();
    }
    else if (name == "textWidth")
    {
        cpu.eax = chance(10) ? 0 : text;
        cpu.edx = random32();
    }
    else if (name == "selectFont")
        cpu.eax = random32();
    else if (name == "measureText" || name == "textWidthWithOverhang")
        cpu.eax = text;
    else if (name == "drawText")
    {
        cpu.eax = text;
        cpu.edx = chance(90) ? below(800) : random32();
        cpu.ebx = chance(90) ? below(600) : random32();
    }
    else if (name == "drawCharacter")
    {
        // A character of either font, a space now and then; its square of
        // the texture, its place, its colour; measured or drawn.
        cpu.eax = chance(15) ? 0xe0 * below(2) : chance(95) ? below(0x1c0) : random32() % 0x400;
        cpu.edx = below(0x100);
        cpu.ebx = below(0x100);
        stack[0] = aPlace();
        stack[1] = chance(90) ? below(600) : random32();
        stack[2] = below(0x40);
        stack[3] = 1 + below(0x40);
        stack[4] = random32();
        stack[5] = chance(60) ? 0 : chance(70) ? 1 : random32();
    }
    else if (name == "takeQuad")
    {
        cpu.eax = 0x7f74c0 + 0x24010 * below(2);
        cpu.edx = kBuffers + 4 * below(0x40);
        cpu.ebx = kBuffers + 0x100 + 4 * below(0x40);
    }
    else if (name == "viewport")
    {
        cpu.eax = kBuffers + 4 * below(0x10);
        cpu.edx = kBuffers + 0x40 + 4 * below(0x10);
        cpu.ebx = kBuffers + 0x80 + 4 * below(0x10);
        cpu.ecx = kBuffers + 0xc0 + 4 * below(0x10);
    }
    else if (name == "toScreenX" || name == "toScreenY" || name == "placePointer" || name == "setPointer")
    {
        cpu.eax = chance(90) ? below(700) - 30 : random32();
        cpu.edx = chance(90) ? below(520) - 20 : random32();
    }
    return true;
}

void makeFrontendArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    if (makeMenusArguments(g, name, cpu, stack))
        return;
    g.word(0x563a80) = kBuffers;
    for (x86::reg32 at = kBuffers; at < kBuffers + 0x1ea0; at += 4)
        g.word(at) = random32() % 0x10000;
    g.word(0x6fd204) = below(8);
    for (x86::reg32 c = 0; c < 8; ++c)
        g.getMemory<x86::reg8>(0x6fc974 + 0x88 * c) = x86::reg8(chance(30) ? 0 : 'A' + below(26));
    static const x86::reg32 ranges[][2] = {
        {0, 0x7a7}, {0x7a7, 0x7a8}, {0x7a8, 0x7bd}, {0x7bd, 0x7ef}, {0x7ef, 0x822}, {0x822, 0x855},
        {0x855, 0x866}, {0x866, 0x900}};
    const auto& range = ranges[below(8)];
    const x86::reg32 number = range[0] + below(range[1] - range[0]);
    if (name == "languageText" || name == "specialText")
        cpu.eax = name == "specialText" && number < 0x7a7 ? 0x7a7 + below(0x200) : number;
    else if (name == "displayModeName")
        cpu.eax = 0x7a8 + below(0x15);
    else if (name == "controllerName")
        cpu.eax = chance(80) ? below(10) : random32() % 0x20;
    else if (name == "nameValid")
    {
        const x86::reg32 text = kStrings + 0x40 * 63;
        const x86::reg32 length = below(14);
        for (x86::reg32 i = 0; i < length; ++i)
            g.getMemory<x86::reg8>(text + i) = x86::reg8('a' + below(26));
        g.getMemory<x86::reg8>(text + length) = 0;
        cpu.eax = chance(10) ? 0 : text;
    }
}

void makeMmxArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    cpu.mmx.mm0 = x86::from_reg64(anMmxValue());
    cpu.mmx.mm1 = x86::from_reg64(anMmxValue());
    cpu.mmx.mm2 = x86::from_reg64(anMmxValue());
    cpu.mmx.mm3 = x86::from_reg64(anMmxValue());
    cpu.mmx.mm4 = x86::from_reg64(anMmxValue());
    cpu.mmx.mm5 = x86::from_reg64(anMmxValue());
    cpu.mmx.mm6 = x86::from_reg64(anMmxValue());
    cpu.mmx.mm7 = x86::from_reg64(anMmxValue());
    makeSamples(g, kMmxState, kMmxState + 0x200);
    const auto frames = [](x86::reg32 count, x86::reg32 block) {
        // As many as a count's loops reach, below 0 and over included.
        return (x86::sreg32(count) < 0 ? 0 - count : count) + 2 * block + 1;
    };
    if (name.compare(0, 3, "add") == 0)
    {
        // (volumes, count, source, accumulator)
        const x86::reg32 block = name == "addStereo16" ? 8 : 4;
        const x86::reg32 count = aCount(block, 0x200);
        const x86::reg32 size = name == "addMono8" ? 1 : name == "addStereo16" ? 4 : 2;
        stack[0] = kMmxState + 8 * below(0x30);
        stack[1] = count;
        stack[2] = kMmxSource + below(0x100);
        stack[3] = kMmxTarget + below(0x100);
        makeSamples(g, stack[2], stack[2] + size * frames(count, block) + 8);
        makeSamples(g, stack[3], stack[3] + 4 * frames(count, block) + 8);
    }
    else if (name.compare(0, 6, "output") == 0)
    {
        // (frames, accumulator, output), from the end: stereo 16 frames at a time.
        const x86::reg32 count = aCount(16, 0x200);
        stack[0] = count;
        stack[1] = kMmxTarget + 0x100 + 4 * below(0x40);
        stack[2] = kMmxOutput + 0x100 + below(0x100);
        makeSamples(g, stack[1] - 0x100, stack[1] + 4 * frames(count, 16));
        makeSamples(g, stack[2] - 0x100, stack[2] + 4 * frames(count, 16));
    }
    else if (name == "clear")
    {
        // (accumulator, dwords): 16 at a time while more than 15 are left
        stack[0] = kMmxTarget + below(0x100);
        stack[1] = aCount(16, 0x400);
        makeSamples(g, stack[0], stack[0] + 4 * frames(stack[1], 16));
    }
    else if (name.compare(0, 11, "interpolate") == 0)
    {
        // (frames, source, output, the whole position's dword, the fraction's,
        // the step's whole, its fraction): positions from 0x200, steps of up
        // to 8 and back by 1, never before the source.
        static const x86::reg32 fractions[] = {0, 0xffffffff, 0x80000000, 0xffff, 0x10000, 1};
        stack[0] = chance(10) ? below(3) : chance(5) ? 0 - (1 + below(4)) : 1 + below(0x100);
        stack[1] = kMmxSource + below(0x100);
        stack[2] = kMmxOutput + below(0x100);
        stack[3] = kMmxState + 0x80 + 4 * below(8);
        stack[4] = kMmxState + 0xa0 + 4 * below(8);
        stack[5] = chance(70) ? below(3) : chance(50) ? 0xffffffff : below(8);
        stack[6] = chance(25) ? fractions[below(6)] : random32();
        g.word(stack[3]) = 0x200 + below(0x100);
        g.word(stack[4]) = chance(25) ? fractions[below(6)] : random32();
        makeSamples(g, stack[1], stack[1] + 4 * (0x300 + 0x100 * 8) + 8);
        makeSamples(g, stack[2], stack[2] + 4 * 0x100 + 8);
    }
    else if (name.compare(0, 2, "xa") == 0)
    {
        // (the decoder's state): samples left, the history, the source and
        // the output; whole blocks of 28 samples, 15 bytes each in mono, 30 in
        // stereo; the table of nibbles the game makes, made up.
        const bool stereo = name == "xaStereo";
        const x86::reg32 state = kMmxState + 0x100 + 4 * below(8);
        const x86::reg32 samples = chance(10) ? 0 : chance(5) ? 0 - below(100) - 1 : chance(30) ? 28 * (1 + below(12))
                                                                                                 : 1 + below(28 * 12);
        const x86::reg32 blocks = x86::sreg32(samples) > 0 ? (samples + 27) / 28 : 0;
        const x86::reg32 source = kMmxSource + below(0x100), target = kMmxTarget + below(0x100);
        g.word(state) = samples;
        g.word(state + (stereo ? 0xc : 8)) = source;
        g.word(state + (stereo ? 0x10 : 0xc)) = target;
        makeSamples(g, source, source + (stereo ? 30 : 15) * blocks + 8);
        makeSamples(g, target, target + (stereo ? 0x70 : 0x38) * blocks + 8);
        for (x86::reg32 at = kXaNibbles; at < kXaNibbles + 0x400; at += 4)
            g.word(at) = random32();
        stack[0] = state;
    }
    else if (name == "copy" || name == "copySwapped")
    {
        // eax to, edx from, ebx bytes -- the other way round for copySwapped:
        // aligned any way, into another buffer or over the source below it, as the
        // memmove calls it: a copy upward over its source goes backwards there.
        const x86::reg32 count = chance(20) ? below(0x24) : chance(15) ? 0x60 + below(0x40) : chance(10)
                                     ? 0x80 * (1 + below(8)) + below(0x40) : below(0x900);
        const x86::reg32 from = kMmxSource + 0x100 + below(0x100);
        const x86::reg32 to = chance(25) ? from - below(0x80) : kMmxTarget + below(0x100);
        makeSamples(g, from - 0x100, from + 0x100 + (x86::sreg32(count) < 0 ? 0 : count));
        if (to >= kMmxTarget)
            makeSamples(g, to, to + 0x10 + (x86::sreg32(count) < 0 ? 0 : count));
        cpu.eax = name == "copy" ? to : from;
        cpu.edx = name == "copy" ? from : to;
        cpu.ebx = count;
    }
}

/* The controls' natives' arguments and worlds (controls_*.inc): each file's
 * function says whether the name was its own. */
#define CONTROLS_ARGUMENTS
#include "controls_input.inc"
#include "controls_devices.inc"
#include "controls_race.inc"
#include "controls_pipeline.inc"
#include "controls_queue.inc"
#undef CONTROLS_ARGUMENTS
#define EA_ARGUMENTS
#include "ea_threads.inc"
#include "ea_memory.inc"
#include "ea_files.inc"
#include "ea_support.inc"
#include "ea_net.inc"
#include "ea_formats.inc"
#undef EA_ARGUMENTS
#define LOAD_ARGUMENTS
#include "load_track.inc"
#include "load_cardb.inc"
#include "load_carmodels.inc"
#include "load_saves.inc"
#undef LOAD_ARGUMENTS
#define FE_ARGUMENTS
#include "frontend_glue.inc"
#include "frontend_draw.inc"
#undef FE_ARGUMENTS
#define DRAW2D_ARGUMENTS
#include "draw2d.inc"
#undef DRAW2D_ARGUMENTS
#define RACESTART_ARGUMENTS
#include "racestart.inc"
#undef RACESTART_ARGUMENTS
#define RACELOAD_ARGUMENTS
#include "raceload.inc"
#undef RACELOAD_ARGUMENTS
#define TRACKDATA_ARGUMENTS
#include "trackdata.inc"
#undef TRACKDATA_ARGUMENTS
#define PHYSICS_ARGUMENTS
#include "physics.inc"
#undef PHYSICS_ARGUMENTS
#define HANDLING_ARGUMENTS
#include "handling.inc"
#undef HANDLING_ARGUMENTS
#define DRIVETRAIN_ARGUMENTS
#include "drivetrain.inc"
#undef DRIVETRAIN_ARGUMENTS
#define CARSTATE_ARGUMENTS
#include "carstate.inc"
#undef CARSTATE_ARGUMENTS
#define TRACK_ARGUMENTS
#include "track.inc"
#undef TRACK_ARGUMENTS
#define COLLIDE_ARGUMENTS
#include "collide.inc"
#undef COLLIDE_ARGUMENTS
#define AIDRIVE_ARGUMENTS
#include "aidrive.inc"
#undef AIDRIVE_ARGUMENTS
#define AI_ARGUMENTS
#include "ai_race.inc"
#undef AI_ARGUMENTS
#define PURSUIT_ARGUMENTS
#include "pursuit.inc"
#undef PURSUIT_ARGUMENTS
#define AIPLAN_ARGUMENTS
#include "aiplan.inc"
#undef AIPLAN_ARGUMENTS
#define SOUND_ARGUMENTS
#include "sound_lib.inc"
#undef SOUND_ARGUMENTS
#define TEXT_ARGUMENTS
#include "text.inc"
#undef TEXT_ARGUMENTS

/* The function's arguments, on the stack at esp + 4 on, and its registers. */
/* The decompiled functions' cases (native_decompiled.cpp): every global the
 * function's code uses (decompiled.json) made up -- 0, 1, -1, small, the top
 * bit set, anything -- on top of the random registers and stack every case
 * has. */
struct DecompiledWorld
{
    const char* name;
    std::vector<x86::reg32> globals;
    std::vector<x86::reg32> values;  // what its code compares with, and one either side
};
const DecompiledWorld kDecompiledWorlds[] = {
#include "decompiled_world.inc"
};

/* THRASH's functions, as the game finds them in the driver by these names
 * (the slots at 0x9ef900 on), for the graphics' cases: each a stand-in by its
 * name, which takes the words its @ says. */
const char* const kThrashNames[] = {
    "_THRASH_drawpointmesh@12", "_THRASH_drawpointstrip@8", "_THRASH_drawlinestrip@8", "_THRASH_drawlinemesh@12",
    "_THRASH_drawpoint@4", "_THRASH_writerect@20", "_THRASH_treset@0", "_THRASH_drawtristrip@8",
    "_THRASH_readrect@20", "_THRASH_drawtrimesh@12", "_THRASH_drawline@8", "_THRASH_clip@16",
    "_THRASH_drawquadmesh@12", "_THRASH_settexture@4", "_THRASH_window@4", "_THRASH_idle@0",
    "_THRASH_pageflip@0", "_THRASH_selectdisplay@4", "_THRASH_sync@4", "_THRASH_drawquad@16",
    "_THRASH_flushwindow@0", "_THRASH_lockwindow@0", "_THRASH_setvideomode@12", "_THRASH_unlockwindow@4",
    "_THRASH_drawtrifan@8", "_THRASH_tupdate@12", "_THRASH_talloc@20", "_THRASH_setstate@8",
    "_THRASH_clearwindow@0", "_THRASH_drawtri@12", "_THRASH_init@0", "_THRASH_restore@0",
    "_THRASH_about@0", "_THRASH_is@0",
};

/* A value as the decompiled functions' cases make one up: 0, 1, -1, small,
 * the top bit set, anything -- so that a register or a global compared with a
 * small number, a state's or a mode's, takes each way. */
x86::reg32 decompiledValue()
{
    const x86::reg32 kind = below(100);
    return kind < 25 ? 0 : kind < 35 ? 1 : kind < 45 ? 0xffffffff : kind < 60 ? below(16)
         : kind < 70 ? below(0x100) : kind < 75 ? 0x80000000 | below(16) : random32();
}

/* The graphics' records, buffers and the driver's descriptions
 * (graphics_world.inc). */
#define GRAPHICS_ARGUMENTS
#include "graphics_world.inc"
#undef GRAPHICS_ARGUMENTS

/* The game's shapes' cases (shapes.inc). */
#define SHAPES_ARGUMENTS
#include "shapes.inc"
#undef SHAPES_ARGUMENTS
#define BUTTONS_ARGUMENTS
#include "buttons.inc"
#undef BUTTONS_ARGUMENTS
#define MENUS_ARGUMENTS
#include "menus.inc"
#undef MENUS_ARGUMENTS
#define LOOP_ARGUMENTS
#include "loop.inc"
#undef LOOP_ARGUMENTS
#define DIALOGS_ARGUMENTS
#include "dialogs.inc"
#undef DIALOGS_ARGUMENTS
#define VIEW_ARGUMENTS
#include "view.inc"
#undef VIEW_ARGUMENTS
#define HUD_ARGUMENTS
#include "hud.inc"
#undef HUD_ARGUMENTS
#define CONV_ARGUMENTS
#include "conv.inc"
#undef CONV_ARGUMENTS

/* The menus' widgets that hold a value rewritten (fields.inc). */
#define FIELDS_ARGUMENTS
#include "fields.inc"
#undef FIELDS_ARGUMENTS
/* The menus' 3D object's cases (obj3d.inc). */
#define OBJ3D_ARGUMENTS
#include "obj3d.inc"
#undef OBJ3D_ARGUMENTS

void makeDecompiledArguments(CheckApp& g, const std::string& name, x86::CPU& cpu)
{
    const DecompiledWorld* world = nullptr;
    for (const DecompiledWorld& w : kDecompiledWorlds)
        if (name == w.name)
            world = &w;
    // A value made up, or now and then one its code compares with.
    auto value = [&]() {
        return world && !world->values.empty() && chance(35) ? world->values[below(x86::reg32(world->values.size()))]
                                                            : decompiledValue();
    };
    // The argument registers made up the same way, for half the cases.
    if (chance(50))
        for (x86::reg32* r : {&cpu.eax, &cpu.edx, &cpu.ebx, &cpu.ecx})
            *r = chance(60) ? value() : *r;
    if (name.compare(0, 10, "graphics::") == 0)
        for (x86::reg32 i = 0; i < sizeof kThrashNames / sizeof *kThrashNames; ++i)
            g.word(kThrashSlots + 4 * i) = chance(97) ? namedStandIn(kThrashNames[i]) : 0;
    for (const DecompiledWorld& w : kDecompiledWorlds)
    {
        if (name != w.name)
            continue;
        for (const x86::reg32 at : w.globals)
            g.word(at) = value();
    }
    // The clock, far from its wrap, and the last flip a few ticks either side
    // of it, which the wait moves on.
    // What an Error calls before its box: none, as the game's data has it
    // until the driver is open -- setVideoMode's Error would call a made-up one.
    if (name == "graphics::setVideoMode")
        g.word(0x564418) = 0;
    // The text drawn: the fonts as the menus' checks make them up.
    if (name == "graphics::drawLoadingAborted")
        makeMenusWorld(g, cpu);
    if (name == "graphics::flipAndWaitFrames")
    {
        g.word(0x79c1a4) = below(0x1000000);
        g.word(0x56415c) = g.word(0x79c1a4) + below(9) - 4;
    }
}

void makeArguments(CheckApp& g, const Tested& t, x86::reg32 esp, x86::CPU& cpu)
{
    x86::reg32 a[16];   // (nine words of the stack's arguments, but for port::hud's, which take up to sixteen)
    for (x86::reg32& w : a)
        w = random32();
    const std::string name = t.name;
    if (name.compare(0, 12, "decompiled::") == 0 || name.compare(0, 10, "graphics::") == 0)
        makeDecompiledArguments(g, name, cpu);
    if (name.compare(0, 10, "graphics::") == 0)
        makeGraphicsArguments(g, name.substr(10), cpu, a);
    if (name == "Error" || name == "Message" || name == "PrintSoundLambda" || name == "TraceSoundLambda"
        || name == "abortmessagewrapper")
    {
        a[0] = chance(10) ? 0 : aString();
    }
    else if (name == "THRASH_opendll")
    {
        cpu.eax = aString();
    }
    else if (name == "audiofocushandler")
    {
        cpu.eax = chance(50) ? 0 : random32();
    }
    else if (name == "iSNDconvertsample")
    {
        static const x86::reg32 representations[] = {0, 2, 3, 3, 1, 4};
        static const x86::reg32 channelCounts[] = {1, 1, 2, 2, 0, 3};
        const x86::reg32 frames = chance(8) ? 0 : chance(5) ? 0xffffffff : 1 + below(0x400);
        const x86::reg32 span = frames == 0xffffffff ? 1 : frames + 1;
        a[0] = kSamples + 4 * below(0x100);
        a[1] = chance(95) ? representations[below(6)] : random32();
        a[2] = chance(95) ? channelCounts[below(6)] : random32();
        a[3] = below(span);
        a[4] = chance(40) ? 0 : chance(10) ? 0xffffffff : a[3] + below(span - a[3]);
        a[5] = frames;
        a[6] = kSamples + 0x8000 + 4 * below(0x100);
        a[7] = chance(80) ? 0 : below(3);
        a[8] = chance(85) ? 1 : below(3);
    }
    makeFillArguments(g, name, cpu);
    if (name.compare(0, 8, "watcom::") == 0)
        makeWatcomArguments(g, name.substr(8), cpu, a);
    if (name.compare(0, 10, "frontend::") == 0 && !makeFeGlueArguments(g, name.substr(10), cpu, a)
        && !makeFeDrawArguments(g, name.substr(10), cpu, a))
        makeFrontendArguments(g, name.substr(10), cpu, a);
    if (name.compare(0, 10, "controls::") == 0)
    {
        const std::string own = name.substr(10);
        if (!makeInputArguments(g, own, cpu, a) && !makeDevicesArguments(g, own, cpu, a)
            && !makeRaceArguments(g, own, cpu, a) && !makePipelineArguments(g, own, cpu, a))
            makeQueueArguments(g, own, cpu, a);
    }
    if (name.compare(0, 6, "load::") == 0)
    {
        const std::string own = name.substr(6);
        if (!makeLoadTrackArguments(g, own, cpu, a) && !makeLoadCardbArguments(g, own, cpu, a)
            && !makeLoadCarmodelsArguments(g, own, cpu, a))
            makeLoadSavesArguments(g, own, cpu, a);
    }
    if (name.compare(0, 4, "ea::") == 0)
    {
        const std::string own = name.substr(4);
        if (!makeEaThreadsArguments(g, own, cpu, a) && !makeEaMemoryArguments(g, own, cpu, a)
            && !makeEaFilesArguments(g, own, cpu, a) && !makeEaSupportArguments(g, own, cpu, a)
            && !makeEaNetArguments(g, own, cpu, a))
            makeEaFormatsArguments(g, own, cpu, a);
    }
    if (name.compare(0, 11, "racestart::") == 0)
        makeRaceStartArguments(g, name.substr(11), cpu, a);
    if (name.compare(0, 10, "raceload::") == 0)
        makeRaceloadArguments(g, name.substr(10), cpu, a);
    if (name.compare(0, 11, "trackdata::") == 0)
        makeTrackdataArguments(g, name.substr(11), cpu, a);
    if (name.compare(0, 9, "physics::") == 0)
        makePhysicsArguments(g, name.substr(9), cpu, a);
    if (name.compare(0, 10, "handling::") == 0)
        makeHandlingArguments(g, name.substr(10), cpu, a);
    if (name.compare(0, 12, "drivetrain::") == 0)
        makeDrivetrainArguments(g, name.substr(12), cpu, a);
    if (name.compare(0, 10, "carstate::") == 0)
        makeCarstateArguments(g, name.substr(10), cpu, a);
    if (name.compare(0, 7, "track::") == 0)
        makeTrackArguments(g, name.substr(7), cpu, a);
    if (name.compare(0, 9, "collide::") == 0)
        makeCollideArguments(g, name.substr(9), cpu, a);
    if (name.compare(0, 9, "aidrive::") == 0)
        makeAidriveArguments(g, name.substr(9), cpu, a);
    if (name.compare(0, 8, "airace::") == 0)
        makeAiRaceArguments(g, name.substr(8), cpu, a);
    if (name.compare(0, 9, "pursuit::") == 0)
        makePursuitArguments(g, name.substr(9), cpu, a);
    if (name.compare(0, 8, "aiplan::") == 0)
        makeAiplanArguments(g, name.substr(8), cpu, a);
    if (name.compare(0, 5, "mix::") == 0)
        makeMmxArguments(g, name.substr(5), cpu, a);
    if (name.compare(0, 5, "snd::") == 0)
        makeSoundArguments(g, name.substr(5), cpu, a);
    // The port's rewrites of the menus' text (text.inc).
    if (name.compare(0, 12, "port::text::") == 0)
        makeTextArguments(g, name.substr(12), cpu, a);
    // The port's rewrites of the sound's: the same world.
    else if (name.compare(0, 6, "port::") == 0)
        makeSoundArguments(g, name.substr(6), cpu, a);
    // The port's 2D drawing: the menus' drawing world.
    if (name.compare(0, 14, "port::draw2d::") == 0)
        makeDraw2dArguments(g, name.substr(14), cpu, a);
    // The game's shapes rewritten (shapes.inc): their own world, over the sound's.
    if (name.compare(0, 14, "port::shapes::") == 0)
        makeShapesArguments(g, name.substr(14), cpu, a);
    // The menus' widgets that hold a value (fields.inc): the menus' drawing world.
    if (name.compare(0, 14, "port::fields::") == 0)
        makeFieldsArguments(g, name.substr(14), cpu, a);
    // The menus' 3D object rewritten (obj3d.inc): its own world, over the sound's.
    if (name.compare(0, 13, "port::obj3d::") == 0)
        makeObj3dArguments(g, name.substr(13), cpu, a);
    // The menus' widgets rewritten (buttons.inc): their own world.
    if (name.compare(0, 15, "port::buttons::") == 0)
        makeButtonsArguments(g, name.substr(15), cpu, a);
    // The menu files rewritten (menus.inc): their own world.
    if (name.compare(0, 13, "port::menus::") == 0)
        makeMenuFilesArguments(g, name.substr(13), cpu, a);
    // The menu loop rewritten (loop.inc): its own world.
    if (name.compare(0, 12, "port::loop::") == 0)
        makeLoopArguments(g, name.substr(12), cpu, a);
    // The dialogs rewritten (dialogs.inc): their own world.
    if (name.compare(0, 15, "port::dialogs::") == 0)
        makeDialogsArguments(g, name.substr(15), cpu, a);
    // The pan-and-zoom view rewritten (view.inc): its own world, over the sound's.
    if (name.compare(0, 12, "port::view::") == 0)
        makeViewArguments(g, name.substr(12), cpu, a);
    // The race HUD rewritten (hud.inc): its own world.
    if (name.compare(0, 11, "port::hud::") == 0)
        makeHudArguments(g, name.substr(11), cpu, a);
    // The functions tools/decomp.py rewrote (conv.inc): a generic world, then their own.
    if (name.compare(0, 12, "port::conv::") == 0)
        makeConvArguments(g, name.substr(12), cpu, a);
    for (x86::reg32 i = 0; i < (name.compare(0, 11, "port::hud::") == 0 || name.compare(0, 12, "port::conv::") == 0 ? 16 : 9); ++i)
        g.word(esp + 4 + 4 * i) = a[i];
}

bool compareRuns(const Tested& t, CheckApp& g, CheckApp& n, const x86::CPU& generatedCpu,
                 const x86::CPU& nativeCpu, x86::reg32 esp, Counts& counts);

/* One case of one function: the state made up on `g`, copied to `n`, then
 * both run and compared. */
/* Cases the generated side of a function never finishes (a loop on a world no game has):
 * GAME_CHECKS_SKIP="name:case,name:case" runs everything of such a case but the two runs
 * themselves, so the cases after it are the cases they always were.  GAME_CHECKS_WATCHDOG=seconds
 * makes a case that takes longer say which it is, and end the process (so that run.py can list
 * them for the skip). */
std::vector<std::pair<std::string, unsigned>> g_skipCases;
std::atomic<long long> g_caseStarted{0};
const char* g_watchName = "";
std::atomic<unsigned> g_watchCase{0};

bool caseSkipped(const char* name, unsigned c)
{
    for (const auto& s : g_skipCases)
        if (s.second == c && s.first == name)
            return true;
    return false;
}

bool runCase(const Tested& t, CheckApp& g, CheckApp& n, Counts& counts)
{
    ++counts.cases;
    const Promise& p = *promiseOf(t.name);
    g_seed = mix(random32() | std::uint64_t(random32()) << 32);
    if (caseSkipped(t.name, g_case))
    {
        // As the case would draw: the stack's words and the registers.
        for (x86::reg32 at = kStackLow; at < kStackHigh; at += 4)
            random32();
        for (int i = 0; i < 7; ++i)
            random32();
        return true;
    }
    g_stopAt = -1;
    g_mostCalls = -1;
    makeState(g);
    if (p.heap != kNoHeap)
        stirHeap(g);
    for (x86::reg32 at = kStackLow; at < kStackHigh; at += 4)
        g.word(at) = random32();
    const x86::reg32 esp = kStackHigh - 0x1000 - 4 * below(64);
    g.word(esp) = 0xdead0000;  // the return address's slot

    x86::CPU cpu{};
    cpu.eax = random32();
    cpu.ebx = random32();
    cpu.ecx = random32();
    cpu.edx = random32();
    cpu.esi = random32();
    cpu.edi = random32();
    cpu.ebp = random32();
    cpu.esp = esp;
    makeArguments(g, t, esp, cpu);

    // The heap's list, for those that leave it as the game would not.
    std::vector<std::pair<x86::reg32, x86::reg32>> list;
    if (p.heap == kRestored)
    {
        for (const x86::reg32 at : {kHeapFirst, kHeapRover, kHeapLargestBefore, kHeapFreeRover})
            list.push_back({at, g.word(at)});
        for (const x86::reg32 heap : miniheaps(g))
            for (const x86::reg32 at : {heap + 4, heap + 8})
                list.push_back({at, g.word(at)});
    }

    g_stopAt = chance(15) ? long(below(p.heap != kNoHeap || chance(50) ? 8 : 40)) : -1;
    if (g_mostCalls >= 0 && (g_stopAt < 0 || g_stopAt > g_mostCalls))
        g_stopAt = g_mostCalls;
    g_lookupFails = chance(30);

    copyWindows(n, g);
    g.calls.clear();
    n.calls.clear();

    /* What the game's code is in now: single precision, to nearest, whatever
     * the case's world asked for (FPU::init, fp:: in include/fpu.h). */
    cpu.fpu.setControl(x86::reg16(0x7f | (cpu.fpu.control.word & 0x1000)));
    /* And what it finds of the machine: MMX's copies chosen, no timestamp counter,
     * no FDIV bug (removed::processorChecked); no MMX registers kept between calls. */
    cpu.mmx.init();
    for (CheckApp* app : {&g, &n})
    {
        app->word(0x5643b4) = 1;
        app->word(0x5643ac) = 1;
        app->word(0x5643bc) = 0;
        app->word(0x5652a4) = 2000000000;  // what its movies ask: their best quality
        app->getMemory<x86::reg8>(0x567858) = 0;
    }
    x86::CPU nativeCpu = cpu, generatedCpu = cpu;
    win32::g_testedPort = std::strncmp(t.name, "port::", 6) == 0;
    win32::g_running = &n;
    t.native(&n, nativeCpu);
    win32::g_running = &g;
    t.generated(&g, generatedCpu);

    /* GAME_CHECKS_CLOBBERS: which registers the game's code leaves otherwise
     * than it got them (eax aside, esp as a ret leaves it), one line a case --
     * what a typed rewrite's adapter must give back besides the result. */
    if (std::getenv("GAME_CHECKS_CLOBBERS") != nullptr && !generatedCpu.terminate)
    {
        std::printf("CLOBBERS %s:", t.name);
        const std::pair<const char*, bool> changed[] = {
            {"ebx", generatedCpu.ebx != cpu.ebx}, {"ecx", generatedCpu.ecx != cpu.ecx},
            {"edx", generatedCpu.edx != cpu.edx}, {"esi", generatedCpu.esi != cpu.esi},
            {"edi", generatedCpu.edi != cpu.edi}, {"ebp", generatedCpu.ebp != cpu.ebp}};
        for (const auto& c : changed)
            if (c.second)
                std::printf(" %s", c.first);
        std::printf("\n");
    }
    const bool alike = compareRuns(t, g, n, generatedCpu, nativeCpu, esp, counts);
    for (const auto& word : list)
        g.word(word.first) = word.second;
    if ((p.name == std::string("watcom::nmalloc") || p.name == std::string("watcom::MemAllocator")
         || p.name == std::string("watcom::nrealloc")
         || (p.name == std::string("watcom::getcwd") && cpu.eax == 0))
        && !generatedCpu.terminate && generatedCpu.eax != 0)
        g_blocks.push_back(generatedCpu.eax);
    keepHeapSound(g);
    return alike;
}

/* The two runs of a case, alike or not. */
bool compareRuns(const Tested& t, CheckApp& g, CheckApp& n, const x86::CPU& generatedCpu,
                 const x86::CPU& nativeCpu, x86::reg32 esp, Counts& counts)
{
    char what[400];
    const bool stopped = generatedCpu.terminate;
    if (stopped)
        ++counts.stopped;
    if (nativeCpu.terminate != generatedCpu.terminate)
        return report(counts, "stopped in one run only");
    if (!(g.calls == n.calls))
    {
        int at = std::snprintf(what, sizeof what, "calls out: %zu generated, %zu native", g.calls.size(),
                               n.calls.size());
        for (size_t i = 0; i < g.calls.size() && i < n.calls.size(); ++i)
            if (!(g.calls[i] == n.calls[i]))
            {
                const Call& a = g.calls[i];
                const Call& b = n.calls[i];
                at += std::snprintf(what + at, sizeof what - at, "; call %zu: %08x (", i, a.id);
                for (size_t j = 0; j < a.words.size() && j < 8; ++j)
                    at += std::snprintf(what + at, sizeof what - at, " %08x", a.words[j]);
                at += std::snprintf(what + at, sizeof what - at, ") generated, %08x (", b.id);
                for (size_t j = 0; j < b.words.size() && j < 8; ++j)
                    at += std::snprintf(what + at, sizeof what - at, " %08x", b.words[j]);
                std::snprintf(what + at, sizeof what - at, ") native");
                break;
            }
        // One run's calls the other's and more: the first of the more.
        const size_t common = std::min(g.calls.size(), n.calls.size());
        bool prefix = true;
        for (size_t i = 0; i < common && prefix; ++i)
            prefix = g.calls[i] == n.calls[i];
        if (prefix && g.calls.size() != n.calls.size())
        {
            const Call& c = g.calls.size() > common ? g.calls[common] : n.calls[common];
            at += std::snprintf(what + at, sizeof what - at, "; call %zu, %s only: %08x (", common,
                                g.calls.size() > common ? "generated" : "native", c.id);
            for (size_t j = 0; j < c.words.size() && j < 8; ++j)
                at += std::snprintf(what + at, sizeof what - at, " %08x", c.words[j]);
            std::snprintf(what + at, sizeof what - at, ")");
        }
        return report(counts, what);
    }
    if (!stopped)
    {
        const Promise& p = *promiseOf(t.name);
        /* The port's rewrites (port::) are held to Watcom's convention, not to
         * the scratch the game's code leaves: the result in eax, every other
         * register as the caller had it -- what the game's callers read (the
         * sound module's callers were checked for it). */
        const bool port = std::string(p.name).compare(0, 6, "port::") == 0;
        const bool kept[] = {false, (!p.watcom || p.arguments < 2) && !port, (!p.watcom || p.arguments < 3) && !port,
                             p.watcom && p.arguments < 4 && !port};  // eax, edx, ebx, ecx as handed over
        const struct { const char* name; x86::reg32 g, n; bool compared; } registers[] = {
            {"eax", generatedCpu.eax, nativeCpu.eax, p.result}, {"ebx", generatedCpu.ebx, nativeCpu.ebx, kept[2]},
            {"ecx", generatedCpu.ecx, nativeCpu.ecx, kept[3]}, {"edx", generatedCpu.edx, nativeCpu.edx, kept[1] && p.watcom},
            {"esi", generatedCpu.esi, nativeCpu.esi, true}, {"edi", generatedCpu.edi, nativeCpu.edi, true},
            {"ebp", generatedCpu.ebp, nativeCpu.ebp, true}, {"esp", generatedCpu.esp, nativeCpu.esp, true},
        };
        for (const auto& r : registers)
            if (r.compared && r.g != r.n && !std::strstr(p.unread, r.name))
            {
                std::snprintf(what, sizeof what, "%s %08x generated, %08x native", r.name, r.g, r.n);
                return report(counts, what);
            }
        if (generatedCpu.fpu.count != nativeCpu.fpu.count)
        {
            std::snprintf(what, sizeof what, "the FPU's stack left otherwise: %u deep generated, %u native",
                          unsigned(generatedCpu.fpu.count), unsigned(nativeCpu.fpu.count));
            return report(counts, what);
        }
        if (p.fpu)
        {
            if (generatedCpu.fpu.control.word != nativeCpu.fpu.control.word)
                return report(counts, "the FPU's control word left otherwise");
            for (x86::reg32 i = 0; i < 8; ++i)
                if (((generatedCpu.fpu.count - i) & 7) < generatedCpu.fpu.count
                    && floatBits(generatedCpu.fpu.regs[i]) != floatBits(nativeCpu.fpu.regs[i]))
                {
                    std::snprintf(what, sizeof what, "the FPU's register %u: %s generated, %s native", unsigned(i),
                                  floatBits(generatedCpu.fpu.regs[i]).c_str(), floatBits(nativeCpu.fpu.regs[i]).c_str());
                    return report(counts, what);
                }
        }
    }
    const std::vector<x86::reg32>& written = pagesEitherWrote(g, n, false);
    for (const auto& w : kWindows)
    {
        const x86::reg32 from = w.first == kStackLow ? esp : w.first;
        const x86::reg32 at = firstDifferenceIn(g, n, from, w.second, written);
        if (at != w.second)
        {
            std::snprintf(what, sizeof what, "memory at %08x: %02x generated, %02x native", at, *bytes(g, at),
                          *bytes(n, at));
            if (std::getenv("DUMPMEM"))
            {
                const x86::reg32 from0 = (at & ~0xfu) - 0x20;
                for (x86::reg32 row = from0; row < from0 + 0x60; row += 0x10)
                {
                    std::printf("    %08x  g:", row);
                    for (x86::reg32 k = 0; k < 16; ++k)
                        std::printf(" %02x", *bytes(g, row + k));
                    std::printf("   n:");
                    for (x86::reg32 k = 0; k < 16; ++k)
                        std::printf(" %02x", *bytes(n, row + k));
                    std::printf("\n");
                }
            }
            return report(counts, what);
        }
    }
    return true;
}

}  // namespace

int main(int argc, char** argv)
{
    const unsigned cases = argc > 1 ? unsigned(std::strtoul(argv[1], nullptr, 10)) : 20000;
    // mutations.py's: stop at the first case that fails, and check one function.
    const bool stopAtFirst = argc > 2 && std::strcmp(argv[2], "--first") == 0;
    const char* only = argc > 3 ? argv[3] : nullptr;
    if (const char* skip = std::getenv("GAME_CHECKS_SKIP"))
    {
        std::string list(skip);
        for (size_t at = 0; at < list.size();)
        {
            size_t end = list.find(',', at);
            if (end == std::string::npos)
                end = list.size();
            const std::string item = list.substr(at, end - at);
            const size_t colon = item.rfind(':');
            if (colon != std::string::npos && colon + 1 < item.size())
                g_skipCases.push_back({item.substr(0, colon), unsigned(std::strtoul(item.c_str() + colon + 1, nullptr, 10))});
            at = end + 1;
        }
    }
    if (const char* watchdog = std::getenv("GAME_CHECKS_WATCHDOG"))
    {
        const long long limit = std::strtoll(watchdog, nullptr, 10) * 1000;
        std::thread([limit] {
            for (;;)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                const long long started = g_caseStarted.load();
                const long long now = std::chrono::duration_cast<std::chrono::milliseconds>(
                                          std::chrono::steady_clock::now().time_since_epoch()).count();
                if (started != 0 && now - started > limit)
                {
                    std::printf("HANG %s:%u\n", g_watchName, g_watchCase.load());
                    std::fflush(stdout);
                    std::_Exit(3);
                }
            }
        }).detach();
    }
    CheckApp g(true), n(false);
    for (CheckApp* app : {&g, &n})
    {
        for (unsigned i = 0; i < kGameSegmentCount; ++i)
            std::memcpy(bytes(*app, kGameSegments[i].address), kGameSegments[i].bytes, kGameSegments[i].length);
        // The imports' slots point at their stand-ins.
        for (unsigned i = 0; i < kPointerCount; ++i)
            if (kPointers[i].address < 0x535000)
                app->word(kPointers[i].address) = kStandIns + 0x10 * i;
        for (x86::reg32 i = 0; i < 64; ++i)
            app->text(kStrings + 0x40 * i, i % 0x3f, mix(i));
    }
    // native_game.cpp's strings are where it says.
    if (std::strcmp(&g.getMemory<char>(0x549c60), "eacsnd.dll") != 0
        || std::strcmp(&g.getMemory<char>(0x54d3a0), "%sa") != 0)
    {
        std::printf("the game's data is not where native_game.cpp has it\n");
        return 2;
    }
    // Nor the runtime's heap where native_watcom_heap.cpp has it: an empty
    // heap, growing by 64K, its lock a bare ret.
    if (g.word(kHeapFirst) != 0 || g.word(kHeapEnabled) != 1 || g.word(kHeapBlockSize) != 0x10000
        || g.word(kHeapAccess) != kNullAccess || g.word(kHeapRelease) != kNullAccess)
    {
        std::printf("the game's data is not where native_watcom_heap.cpp has it\n");
        return 2;
    }

    for (const Tested& t : kTested)
        if (!promiseOf(t.name))
        {
            std::printf("%s: no promise of its registers here\n", t.name);
            return 2;
        }

    bool failed = false;
    for (const Tested& t : kTested)
    {
        if (only && std::strcmp(only, t.name) != 0)
            continue;
        Counts counts;
        g_name = t.name;
        for (g_case = 0; g_case < cases; ++g_case)
        {
            g_watchName = t.name;
            g_watchCase = g_case;
            g_caseStarted = std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::steady_clock::now().time_since_epoch()).count();
            if (!runCase(t, g, n, counts) && stopAtFirst)
                return 1;
            // Both memories alike again, the dead stack included, for the next case.
            copyWindows(n, g);
            if (g_case % 2000 == 1999 || g_case + 1 == cases)
            {
                const x86::reg32 at = firstDifference(g, n, 0, kMemory);
                if (at != kMemory)
                {
                    std::printf("  %s: memory at %08x written outside the windows\n", t.name, at);
                    ++counts.failed;
                    std::memcpy(bytes(n, 0), bytes(g, 0), kMemory);
                    copyWindows(n, g);
                }
            }
        }
        std::printf("%-34s %7u cases, %5u stopped the game -- %u failed\n", t.name, counts.cases, counts.stopped,
                    counts.failed);
        g_caseStarted = 0;
        failed = failed || counts.failed != 0;
    }
    std::printf(failed ? "FAILED\n" : "all alike\n");
    return failed ? 1 : 0;
}
