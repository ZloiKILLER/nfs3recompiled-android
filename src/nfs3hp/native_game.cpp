#include "native_guest.h"
#include <cstring>
#include <iterator>

/* nfs3.exe's own functions the port has natively, where their code was:
 * disassemble_nfs3hp.py binds each address to the function of that name here
 * (GAME_NATIVES), and the generated code calls it by name: the game's side
 * of its two driver interfaces, THRASH and iSND, the logger they report
 * through, the functions of the game's that iSNDloaddll hands eacsnd, and the
 * fill the game's code clears and fills its memory with.  The names are EA's
 * where the game's own messages give them.
 * tools/game_checks generates the game's code for them afresh and holds each
 * one against it.
 *
 * Each keeps what its caller counts on: the game's own callers are Watcom
 * code, which counts on every register it handed no argument in (and the
 * functions here keep even those, but eax, as the game's code did); eacsnd
 * calls the lambdas stdcall or cdecl, and counts on ebx, esi, edi and ebp.  They call what the
 * game's code called as it did -- the arguments where it pushed them, in the
 * registers it had them in, and the stack where it had it, which is where a
 * pointer into its frame points.  The game's own functions they call stay
 * where they are, generated, called through dynamic_call. */

namespace nfs3hp
{

void commMessageTrace(win32::WinApplication* app, x86::CPU& cpu);  // native_vertices.cpp

void Error(win32::WinApplication* app, x86::CPU& cpu);
void Message(win32::WinApplication* app, x86::CPU& cpu);
void AcquireWindow(win32::WinApplication* app, x86::CPU& cpu);
void iSNDloaddll(win32::WinApplication* app, x86::CPU& cpu);
void SNDcaps(win32::WinApplication* app, x86::CPU& cpu);

namespace
{

using guest::at;
using guest::call;
using guest::put;

/* The game's imports (their slots), as its code calls them. */
constexpr x86::reg32 kDebugBreak = 0x5344a8;
constexpr x86::reg32 kEnterCriticalSection = 0x5344b8;
constexpr x86::reg32 kExitProcess = 0x5344c0;
constexpr x86::reg32 kGetProcAddress = 0x534558;
constexpr x86::reg32 kLeaveCriticalSection = 0x534588;
constexpr x86::reg32 kLoadLibraryA = 0x53458c;
constexpr x86::reg32 kOutputDebugStringA = 0x53459c;
constexpr x86::reg32 kVirtualAlloc = 0x534624;
constexpr x86::reg32 kVirtualFree = 0x534628;
constexpr x86::reg32 kMessageBoxA = 0x534764;
constexpr x86::reg32 kShowCursor = 0x534794;

/* Watcom's C library in the game: register calls (eax, edx, ebx), but for
 * sprintf. */
constexpr x86::reg32 kStdout = 0x566f72;    // the FILE, __iob[1]
constexpr x86::reg32 kVfprintf = 0x4ff014;  // (stream, format, the va_list's address)
constexpr x86::reg32 kVsprintf = 0x4df674;  // (buffer, format, the va_list's address)
constexpr x86::reg32 kSprintf = 0x4df690;   // cdecl (buffer, format, ...)
constexpr x86::reg32 kFflush = 0x4f7d70;    // (stream)

/* Where an Error comes from, for its text: each holds a file's name, a
 * function's and a line, in that order, set before the call. */
constexpr x86::reg32 kWhere = 0x552190;
constexpr x86::reg32 kCalledFrom = 0x55219c;  // what called that, where somebody says

constexpr x86::reg32 kWindow = 0x9f3d24;  // the game's window

}  // namespace

/* AcquireWindow (0x4eba70): the game's window. */
void AcquireWindow(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.eax = at(app, kWindow);
    cpu.esp += 4;
}

/* ---- The logger ---- */

namespace
{

constexpr x86::reg32 kErrorUnderWay = 0x5521a8;  // and another Error does nothing
constexpr x86::reg32 kErrorText = 0x56e010;
constexpr x86::reg32 kWhereFormat = 0x536004;       // "%s(%d) %s()\n"
constexpr x86::reg32 kCalledFromFormat = 0x536014;  // "CALLED FROM %s(%d) %s()\n"
constexpr x86::reg32 kErrorCaption = 0x536030;      // "Abort message:"
constexpr x86::reg32 kJustText = 0x536040;          // "%s"
/* What the game sets about its errors. */
constexpr x86::reg32 kDebugOutput = 0x5643fc;  // the text to OutputDebugStringA as well
constexpr x86::reg32 kDebugStop = 0x55e8c0;    // and DebugBreak after it
constexpr x86::reg32 kErrorFilter = 0x564414;  // asked first, eax the text: 0 and the error is let go
constexpr x86::reg32 kErrorWhere = 0x564408;   // where it came from after the text
constexpr x86::reg32 kErrorLock = 0x564418;    // called before the box: a critical section, left after it
constexpr x86::reg32 kErrorBefore = 0x56441c;  // called before the box, after that
constexpr x86::reg32 kErrorPrintOk = 0x564400; // OK pressed: the text to stdout, then kErrorOk called
constexpr x86::reg32 kErrorOk = 0x564404;
/* The game's way out. */
constexpr x86::reg32 kOnMainThread = 0x4df930;  // eax 0: whether this is the game's main thread
constexpr x86::reg32 kExit = 0x4dfd04;          // exit(eax), the game's

/* `format` with the file, line and function at `where` into `buffer`, then
 * on the end of Error's text, as Error's code does it -- the copy inline. */
bool addWhere(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buffer, x86::reg32 where, x86::reg32 format)
{
    if (!call(app, cpu, kSprintf, {buffer, format, at(app, where), at(app, where + 8), at(app, where + 4)}))
        return false;
    x86::reg32 to = kErrorText;
    while (app->getMemory<x86::reg8>(to) != 0)
        ++to;
    for (x86::reg32 from = buffer;; ++from, ++to)
    {
        const x86::reg8 c = app->getMemory<x86::reg8>(from);
        app->getMemory<x86::reg8>(to) = c;
        if (c == 0)
            return true;
    }
}

}  // namespace

/* Error (0x401010): cdecl (format, ...), keeps every register but eax.  The
 * game's abort: the text formatted, where it came from added, a message box,
 * and the game ended -- by its own exit on its main thread, ExitProcess on any
 * other.  Only one at a time; the game may let one go (kErrorFilter), and
 * then the next is shown. */
void Error(win32::WinApplication* app, x86::CPU& cpu)
{
    commMessageTrace(app, cpu);
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 ebx = cpu.ebx, ecx = cpu.ecx, edx = cpu.edx, esi = cpu.esi, edi = cpu.edi, ebp = cpu.ebp;
    const auto back = [&]() {
        cpu.ebx = ebx;
        cpu.ecx = ecx;
        cpu.edx = edx;
        cpu.esi = esi;
        cpu.edi = edi;
        cpu.ebp = ebp;
        cpu.esp = esp + 4;
    };
    if (at(app, kErrorUnderWay) != 0)
    {
        back();
        return;
    }
    put(app, kErrorUnderWay, 1);
    const x86::reg32 frame = esp - 0x18;     // below the six registers it keeps
    const x86::reg32 buffer = frame - 0x88;  // for where it came from; where esp is
    cpu.esp = buffer;
    const x86::reg32 format = at(app, esp + 4);
    if (format != 0)
    {
        const x86::reg32 arguments = frame - 8;  // its va_list
        put(app, arguments, esp + 8);
        cpu.eax = kErrorText;
        cpu.edx = format;
        cpu.ebx = arguments;
        if (!call(app, cpu, kVsprintf))
            return;
    }
    else
    {
        app->getMemory<x86::reg8>(kErrorText) = 0;
    }
    if (at(app, kDebugOutput) != 0)
    {
        if (!call(app, cpu, at(app, kOutputDebugStringA), {kErrorText}))
            return;
        if (at(app, kDebugStop) != 0 && !call(app, cpu, at(app, kDebugBreak)))
            return;
    }
    if (const x86::reg32 filter = at(app, kErrorFilter))
    {
        cpu.eax = kErrorText;
        if (!call(app, cpu, filter))
            return;
        if (cpu.eax == 0)
        {
            put(app, kErrorUnderWay, 0);
            back();
            return;
        }
    }
    if (at(app, kErrorWhere) != 0)
    {
        if (at(app, kWhere) != 0 && !addWhere(app, cpu, buffer, kWhere, kWhereFormat))
            return;
        if (at(app, kCalledFrom) != 0 && !addWhere(app, cpu, buffer, kCalledFrom, kCalledFromFormat))
            return;
    }
    x86::reg32 lock = 0;
    if (const x86::reg32 before = at(app, kErrorLock))
    {
        if (!call(app, cpu, before))
            return;
        lock = cpu.eax;
    }
    if (const x86::reg32 before = at(app, kErrorBefore); before != 0 && !call(app, cpu, before))
        return;
    if (!call(app, cpu, at(app, kShowCursor), {1}))
        return;
    // MB_OKCANCEL | MB_ICONEXCLAMATION | MB_SYSTEMMODAL | MB_SETFOREGROUND
    if (!call(app, cpu, at(app, kMessageBoxA), {0, kErrorText, kErrorCaption, 0x11031}))
        return;
    if (cpu.eax == 1 && at(app, kErrorPrintOk) != 0 && at(app, kErrorOk) != 0)
    {
        if (!call(app, cpu, Message, {kJustText, kErrorText}) || !call(app, cpu, at(app, kErrorOk)))
            return;
    }
    if (lock != 0 && !call(app, cpu, at(app, kLeaveCriticalSection), {lock}))
        return;
    cpu.eax = 0;
    if (!call(app, cpu, kOnMainThread))
        return;
    if (cpu.eax != 0)
    {
        cpu.eax = 0;
        if (!call(app, cpu, kExit))
            return;
    }
    else if (!call(app, cpu, at(app, kExitProcess), {cpu.eax}))
    {
        return;
    }
    back();
}

/* Message (0x4df6c0): cdecl (format, ...), printf -- vfprintf to stdout;
 * keeps ebx and edx, which vfprintf does not. */
void Message(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 ebx = cpu.ebx, edx = cpu.edx;
    const x86::reg32 arguments = esp - 0xc;  // its va_list, below the two registers it keeps
    put(app, arguments, esp + 8);
    cpu.eax = kStdout;
    cpu.edx = at(app, esp + 4);
    cpu.ebx = arguments;
    cpu.esp = arguments;
    if (!call(app, cpu, kVfprintf))
        return;
    cpu.ebx = ebx;
    cpu.edx = edx;
    cpu.esp = esp + 4;
}

/* ---- iSND, the game's side: eacsnd loaded and called ---- */

namespace
{

constexpr x86::reg32 kSoundModule = 0x9ef600;   // eacsnd.dll's handle
constexpr x86::reg32 kSoundOptions = 0x9ef608;  // what the sound is started with
/* iSNDloaddll's lookups after setfunctions, into 0x9ef61c.. in this order. */
constexpr x86::reg32 kSoundFunctions = 0x9ef61c;
constexpr x86::reg32 kSoundCaps = 0x9ef61c;   // iSNDdirectcaps
constexpr x86::reg32 kSoundStart = 0x9ef620;  // iSNDdirectstart
constexpr x86::reg32 kSoundStop = 0x9ef624;   // iSNDdirectstop
constexpr x86::reg32 kSoundServe = 0x9ef634;  // iSNDdirectserve
constexpr x86::reg32 kSoundFunctionNames[] = {
    0x549db0, 0x549dc4, 0x549dd8, 0x549dec, 0x549e08, 0x549e24, 0x549e3c,
    0x549e50, 0x549e6c, 0x549e84, 0x549e9c, 0x549eb0, 0x549ec4, 0x549ed8,
};
constexpr x86::reg32 kSoundLock = 0xa09b04;     // a critical section's address: around start, stop and serve
constexpr x86::reg32 kSoundTick = 0xa09b08;     // the 100 Hz tick TickSoundLambda calls
constexpr x86::reg32 kGameTick = 0x500c80;      // what it is unless the game set another
constexpr x86::reg32 kSoundVersion = 0x100003;  // 16.3, what iSNDdllversion must say
/* What iSNDloaddll hands eacsnd's setfunctions, in its order: the natives
 * below, where the game's code was. */
constexpr x86::reg32 kSoundLambdas[] = {
    0x4e1a24,  // AcquireSoundDataLambda
    0x4e1a00,  // StopSoundBufferLambda
    0x4e1a88,  // abortmessagewrapper
    0x4e1a64,  // TraceSoundLambda
    0x4e1adc,  // TickSoundLambda
    0x4e1a34,  // PrintSoundLambda
    0x4e1a0c,  // AcquireSoundBufferPositionLambda
    0x4e1ae4,  // iSNDconvertsample
};
/* The game's sound functions the lambdas hand on to. */
constexpr x86::reg32 kMix = 0x500864;               // (eax buffer, edx frames)
constexpr x86::reg32 kStopBuffer = 0x4ffb64;        // (eax buffer)
constexpr x86::reg32 kBufferPosition = 0x4ffce0;    // (eax, edx, ebx position)
constexpr x86::reg32 kTrace = 0x4e08f0;             // cdecl (format, ...), into the game's log
/* Strings: the file and functions iSND's errors are from, and the texts. */
constexpr x86::reg32 kSoundSource = 0x549b10;       // "win\sdriver.c"
constexpr x86::reg32 kAbortmessagewrapperName = 0x549b20;
constexpr x86::reg32 kConvertName = 0x549b34;       // "iSNDconvertsample"
constexpr x86::reg32 kFocusName = 0x549c18;         // "audiofocushandler"
constexpr x86::reg32 kLoadName = 0x549c6c;          // "iSNDloaddll"
constexpr x86::reg32 kCapsName = 0x549f20;          // "SNDcaps"
constexpr x86::reg32 kSoundDll = 0x549c60;          // "eacsnd.dll"
constexpr x86::reg32 kVersionName = 0x549cd8;       // "_iSNDdllversion@0"
constexpr x86::reg32 kSetFunctionsName = 0x549d68;  // "_iSNDdirectsetfunctions@32"

/* Error(arguments...) as iSND's code calls it: in "win\sdriver.c",
 * `function`, `line`. */
bool soundError(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 function, x86::reg32 line,
                std::initializer_list<x86::reg32> arguments)
{
    put(app, kWhere, kSoundSource);
    put(app, kWhere + 4, function);
    put(app, kWhere + 8, line);
    return call(app, cpu, Error, arguments);
}

/* iSNDloaddll's work: false when the game stopped, else `result` 0, -1 for
 * what it could not find, -11 for the wrong version. */
bool loadSound(win32::WinApplication* app, x86::CPU& cpu, x86::reg32& result)
{
    result = 0xffffffff;
    if (!call(app, cpu, at(app, kLoadLibraryA), {kSoundDll}))
        return false;
    const x86::reg32 module = cpu.eax;
    put(app, kSoundModule, module);
    if (module == 0)  // "COULDN'T LOAD/FIND EACSND.DLL..."
        return soundError(app, cpu, kLoadName, 0x153, {0x549c78});
    if (!call(app, cpu, at(app, kGetProcAddress), {module, kVersionName}))
        return false;
    const x86::reg32 version = cpu.eax;
    if (version == 0)  // "COULDN'T FIND iSNDdllversion."
        return soundError(app, cpu, kLoadName, 0x15f, {0x549cec});
    if (!call(app, cpu, version))
        return false;
    if (cpu.eax != kSoundVersion)
    {
        // "EACSND.DLL VERSION MISMATCH. REQUIRED %i.%i, FOUND %i.%i."
        result = 0xfffffff5;
        return soundError(app, cpu, kLoadName, 0x169,
                          {0x549d1c, 0x10, 3, x86::reg32(x86::sreg32(cpu.eax) >> 16), cpu.eax & 0xffff});
    }
    if (!call(app, cpu, at(app, kGetProcAddress), {at(app, kSoundModule), kSetFunctionsName}))
        return false;
    const x86::reg32 setFunctions = cpu.eax;
    if (setFunctions == 0)  // "COULDN'T FIND setfunctions."
        return soundError(app, cpu, kLoadName, 0x176, {0x549d84});
    const x86::reg32* l = kSoundLambdas;
    if (!call(app, cpu, setFunctions, {l[0], l[1], l[2], l[3], l[4], l[5], l[6], l[7]}))
        return false;
    x86::reg32 slot = kSoundFunctions;
    for (const x86::reg32 name : kSoundFunctionNames)
    {
        if (!call(app, cpu, at(app, kGetProcAddress), {at(app, kSoundModule), name}))
            return false;
        put(app, slot, cpu.eax);
        slot += 4;
    }
    for (slot = kSoundFunctions; slot < kSoundFunctions + 4 * std::size(kSoundFunctionNames); slot += 4)
        if (at(app, slot) == 0)  // "COULDN'T FIND NEEDED FUNCTION."
            return soundError(app, cpu, kLoadName, 0x1ad, {0x549ef0});
    result = 0;
    return true;
}

}  // namespace

/* iSNDloaddll (0x4e1dd4): eacsnd.dll loaded, its version checked, the game's
 * functions handed it and its own looked up; 0 if it was already, -1 or -11
 * after the Error.  Keeps every register but eax. */
void iSNDloaddll(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 ebx = cpu.ebx, ecx = cpu.ecx, edx = cpu.edx, esi = cpu.esi, edi = cpu.edi, ebp = cpu.ebp;
    x86::reg32 result = 0;
    if (at(app, kSoundModule) == 0)
    {
        cpu.esp = esp - 0x18;  // below the six registers it keeps
        if (!loadSound(app, cpu, result))
            return;
    }
    cpu.eax = result;
    cpu.ebx = ebx;
    cpu.ecx = ecx;
    cpu.edx = edx;
    cpu.esi = esi;
    cpu.edi = edi;
    cpu.ebp = ebp;
    cpu.esp = esp + 4;
}

/* SNDcaps (0x4e21b0): what the card can, as iSNDdirectcaps says for the
 * game's window, 0x10000 added if that is above 0; eacsnd loaded first, and
 * the tick the game's own unless it set another.  Keeps every register but
 * eax. */
void SNDcaps(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 ecx = cpu.ecx, edx = cpu.edx;
    if (at(app, kSoundTick) == 0)
        put(app, kSoundTick, kGameTick);
    cpu.esp = esp - 8;  // below the two registers it keeps
    if (!call(app, cpu, iSNDloaddll) || !call(app, cpu, AcquireWindow))
        return;
    const x86::reg32 window = cpu.eax;
    if (window == 0)
    {
        cpu.esp = esp - 0x14;  // below ebx, esi and edi too, which it keeps around the call
        // "SNDcaps - MAIN WINDOW HANDLE IS INVALID."
        if (!soundError(app, cpu, kCapsName, 0x22d, {0x549f28}))
            return;
        cpu.esp = esp - 8;
    }
    if (!call(app, cpu, at(app, kSoundCaps), {window}))
        return;
    cpu.eax = x86::sreg32(cpu.eax) > 0 ? cpu.eax | 0x10000 : cpu.eax;
    cpu.ecx = ecx;
    cpu.edx = edx;
    cpu.esp = esp + 4;
}

/* Is3DSoundAvailable (0x40faf0): 1 if SNDcaps has 0x20000, else 0.  (It hands
 * SNDcaps -1 in eax, which SNDcaps never reads.) */
void Is3DSoundAvailable(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    cpu.esp = esp - 4;  // below ebp, which it keeps
    if (!call(app, cpu, SNDcaps))
        return;
    cpu.eax = (cpu.eax & 0x20000) != 0 ? 1 : 0;
    cpu.esp = esp + 4;
}

/* audiofocushandler (0x4e1d48): eax nonzero, the game has the focus and the
 * sound is started for its window with kSoundOptions; zero, it is stopped.
 * Under the sound's lock; keeps every register but eax. */
void audiofocushandler(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 ebx = cpu.ebx, ecx = cpu.ecx, edx = cpu.edx, ebp = cpu.ebp;
    const x86::reg32 focus = cpu.eax;
    cpu.esp = esp - 0x10;  // below the four registers it keeps
    if (!call(app, cpu, at(app, kEnterCriticalSection), {at(app, kSoundLock)}))
        return;
    if (focus != 0)
    {
        if (!call(app, cpu, AcquireWindow))
            return;
        const x86::reg32 window = cpu.eax;
        if (window == 0)
        {
            cpu.esp = esp - 0x18;  // below edi and esi too, which it keeps around the call
            // "audiofocushandler - MAIN WINDOW HANDLE IS INVALID."
            if (!soundError(app, cpu, kFocusName, 0x124, {0x549c2c}))
                return;
            cpu.esp = esp - 0x10;
        }
        if (!call(app, cpu, at(app, kSoundStart), {at(app, kSoundOptions), window}))
            return;
    }
    else if (!call(app, cpu, at(app, kSoundStop)))
    {
        return;
    }
    if (!call(app, cpu, at(app, kLeaveCriticalSection), {at(app, kSoundLock)}))
        return;
    cpu.ebx = ebx;
    cpu.ecx = ecx;
    cpu.edx = edx;
    cpu.ebp = ebp;
    cpu.esp = esp + 4;
}

/* SoundServeHandler (0x4e2234): iSNDdirectserve under the sound's lock;
 * keeps every register but eax. */
void SoundServeHandler(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 ecx = cpu.ecx, edx = cpu.edx;
    cpu.esp = esp - 8;  // below the two registers it keeps
    if (!call(app, cpu, at(app, kEnterCriticalSection), {at(app, kSoundLock)})
        || !call(app, cpu, at(app, kSoundServe))
        || !call(app, cpu, at(app, kLeaveCriticalSection), {at(app, kSoundLock)}))
        return;
    cpu.ecx = ecx;
    cpu.edx = edx;
    cpu.esp = esp + 4;
}

/* The lambdas iSNDloaddll hands eacsnd (kSoundLambdas), each to the game's
 * own function behind it: stdcall, and the three that format, cdecl. */

/* AcquireSoundDataLambda (0x4e1a24): stdcall (buffer, frames), the mix. */
void AcquireSoundDataLambda(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    cpu.eax = at(app, esp + 4);
    cpu.edx = at(app, esp + 8);
    if (!call(app, cpu, kMix))
        return;
    cpu.esp = esp + 4 + 8;
}

/* StopSoundBufferLambda (0x4e1a00): stdcall (buffer). */
void StopSoundBufferLambda(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    cpu.eax = at(app, esp + 4);
    if (!call(app, cpu, kStopBuffer))
        return;
    cpu.esp = esp + 4 + 4;
}

/* AcquireSoundBufferPositionLambda (0x4e1a0c): stdcall (three words, the
 * last where the position goes); keeps ebx. */
void AcquireSoundBufferPositionLambda(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 ebx = cpu.ebx;
    cpu.eax = at(app, esp + 4);
    cpu.edx = at(app, esp + 8);
    cpu.ebx = at(app, esp + 0xc);
    cpu.esp = esp - 4;  // below ebx
    if (!call(app, cpu, kBufferPosition))
        return;
    cpu.ebx = ebx;
    cpu.esp = esp + 4 + 0xc;
}

/* TickSoundLambda (0x4e1adc): the game's 100 Hz tick (kSoundTick). */
void TickSoundLambda(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    if (!call(app, cpu, at(app, kSoundTick)))
        return;
    cpu.esp = esp + 4;
}

/* PrintSoundLambda (0x4e1a34): cdecl (format, ...), to stdout, flushed --
 * Message handed the format and the address of a va_list of the rest, as if
 * it were the first of them. */
void PrintSoundLambda(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 arguments = esp - 4;
    put(app, arguments, esp + 8);
    cpu.esp = arguments;
    if (!call(app, cpu, Message, {at(app, esp + 4), arguments}))
        return;
    cpu.eax = kStdout;
    if (!call(app, cpu, kFflush))
        return;
    cpu.esp = esp + 4;
}

/* TraceSoundLambda (0x4e1a64): cdecl (format, ...), into the game's log as
 * PrintSoundLambda hands Message its arguments. */
void TraceSoundLambda(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 arguments = esp - 4;
    put(app, arguments, esp + 8);
    cpu.esp = arguments;
    if (!call(app, cpu, kTrace, {at(app, esp + 4), arguments}))
        return;
    cpu.esp = esp + 4;
}

/* abortmessagewrapper (0x4e1a88): cdecl (text, ...), stdout flushed and the
 * text an Error from this function -- with the address of a va_list of the
 * rest as its one argument. */
void abortmessagewrapper(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 arguments = esp - 0x10;  // its va_list, below ebx, esi and edi, which it keeps
    cpu.esp = arguments;
    cpu.eax = kStdout;
    if (!call(app, cpu, kFflush))
        return;
    put(app, arguments, esp + 8);
    if (!soundError(app, cpu, kAbortmessagewrapperName, 0x92, {at(app, esp + 4), arguments}))
        return;
    cpu.esp = esp + 4;
}

/* iSNDconvertsample (0x4e1ae4): stdcall (source, its representation, its
 * channels, first, last, frames, destination, its representation, its
 * channels), a sample into the game's own form.  The source is converted
 * into a buffer of VirtualAlloc's -- 8 bits (0), 16 (2) or XA (3, one channel
 * or two) -- one channel made two, then from first to last (all of it, when
 * last is not above 0) into the destination; only one destination channel
 * and representation 0 are done, the rest is an Error.  The buffer freed is
 * where the frames started, first's frame when there is a last: the game's
 * own slip.  0. */
void iSNDconvertsample(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    const auto argument = [&](x86::reg32 i) { return at(app, esp + 4 * i); };
    const x86::reg32 source = argument(1), representation = argument(2), channels = argument(3);
    const x86::reg32 frames = argument(6);
    const x86::reg32 frame = esp - 0x40;  // below the four registers it keeps, and its 0x30 bytes
    cpu.esp = frame;
    // "CAN'T CONVERT XA WITH %i CHANNELS."
    if (x86::sreg32(channels) > 2 && !soundError(app, cpu, kConvertName, 0xaa, {0x549b48, channels}))
        return;
    // "CAN'T HANDLE dstnumchan %i"
    if (argument(9) != 1 && !soundError(app, cpu, kConvertName, 0xb0, {0x549b80, argument(9)}))
        return;
    if (!call(app, cpu, at(app, kVirtualAlloc), {0, frames * 8 + 0x4e20, 0x3000, 4}))
        return;
    x86::reg32 buffer = cpu.eax;
    switch (representation)
    {
    case 0:
    case 2:
        if (!call(app, cpu, representation == 0 ? 0x5008a0 : 0x500b10, {frames * channels, source, buffer}))
            return;
        break;
    case 3:
        // XA: a record in its frame -- the frames, zeros, the source and the
        // buffer -- for the one channel's decoder or the two's.
        if (channels == 1)
        {
            const x86::reg32 record = frame + 0x1c;
            put(app, record, frames);
            put(app, record + 4, 0);
            put(app, record + 8, 0);
            put(app, record + 0xc, source);
            put(app, record + 0x10, buffer);
            if (!call(app, cpu, 0x500b90, {record}))
                return;
        }
        else if (channels == 2)
        {
            const x86::reg32 record = frame;
            put(app, record, frames);
            for (x86::reg32 offset = 4; offset < 0x14; offset += 4)
                put(app, record + offset, 0);
            put(app, record + 0x14, source);
            put(app, record + 0x18, buffer);
            if (!call(app, cpu, 0x500990, {record}))
                return;
        }
        break;
    default:
        // "CAN'T HANDLE srcsamplerep %i"
        if (!soundError(app, cpu, kConvertName, 0xe0, {0x549bb0, representation}))
            return;
        break;
    }
    if (channels == 1)
    {
        // One channel made two, from the end back: each sample as it was, then
        // through the x87.
        for (x86::sreg32 i = x86::sreg32(frames) - 1; i >= 0; --i)
        {
            const x86::reg32 from = buffer + 4 * x86::reg32(i), to = buffer + 8 * x86::reg32(i);
            const x86::reg32 bits = at(app, from);
            app->getMemory<float>(to + 4) = float(x86::Float(app->getMemory<float>(from)));
            put(app, to, bits);
        }
    }
    x86::reg32 count = frames;
    if (x86::sreg32(argument(5)) > 0)
    {
        count = argument(5) - argument(4) + 1;
        buffer += argument(4) << 3;
    }
    if (argument(8) != 0)
    {
        // "CAN'T HANDLE dstsamplerep %i"
        if (!soundError(app, cpu, kConvertName, 0x101, {0x549be4, argument(8)}))
            return;
    }
    else if (!call(app, cpu, 0x500910, {count, buffer, argument(7)}))
    {
        return;
    }
    if (!call(app, cpu, at(app, kVirtualFree), {buffer, 0, 0x8000}))  // MEM_RELEASE
        return;
    cpu.eax = 0;
    cpu.esp = esp + 4 + 0x24;
}

/* ---- THRASH, the game's side ---- */

namespace
{

constexpr x86::reg32 kRendererModule = 0x564a6c;
constexpr x86::reg32 kRendererIs3dfx = 0x564410;  // THRASH_about's record starts "XFD3"
constexpr x86::reg32 kThrashVersion = 0x68;       // 104, what THRASH_about must say
/* THRASH_opendll's lookups, in its order: the name, and where the pointer goes. */
constexpr struct
{
    x86::reg32 name, slot;
} kThrashFunctions[] = {
    {0x54d3a4, 0x9ef980}, {0x54d3b4, 0x9ef970}, {0x54d3cc, 0x9ef92c}, {0x54d3dc, 0x9ef928}, {0x54d3f0, 0x9ef90c},
    {0x54d408, 0x9ef908}, {0x54d420, 0x9ef910}, {0x54d434, 0x9ef904}, {0x54d450, 0x9ef900}, {0x54d46c, 0x9ef94c},
    {0x54d480, 0x9ef930}, {0x54d498, 0x9ef974}, {0x54d4ac, 0x9ef960}, {0x54d4c4, 0x9ef924}, {0x54d4dc, 0x9ef91c},
    {0x54d4f4, 0x9ef950}, {0x54d50c, 0x9ef93c}, {0x54d51c, 0x9ef978}, {0x54d52c, 0x9ef984}, {0x54d53c, 0x9ef954},
    {0x54d554, 0x9ef940}, {0x54d568, 0x9ef920}, {0x54d57c, 0x9ef97c}, {0x54d590, 0x9ef944}, {0x54d5a8, 0x9ef96c},
    {0x54d5bc, 0x9ef934}, {0x54d5d4, 0x9ef958}, {0x54d5ec, 0x9ef948}, {0x54d5fc, 0x9ef968}, {0x54d610, 0x9ef918},
    {0x54d624, 0x9ef964}, {0x54d638, 0x9ef95c}, {0x54d650, 0x9ef938}, {0x54d664, 0x9ef914},
};
constexpr x86::reg32 kThrashAbout = 0x9ef980;     // _THRASH_about@0
constexpr x86::reg32 kThrashSetState = 0x9ef96c;  // _THRASH_setstate@8

}  // namespace

/* THRASH_opendll (0x4f8390): eax the renderer's name -- "voodoo2" -- and that
 * with an "a" after it if that will not load; its 34 functions looked up,
 * state 0x1f set to 104, and THRASH_about asked whether it is 3dfx's and of
 * version 104.  1 if it loaded and is (or will not say), else 0, after a
 * Message when it is not.  Keeps every register but eax. */
void THRASH_opendll(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 ebx = cpu.ebx, ecx = cpu.ecx, edx = cpu.edx, esi = cpu.esi, ebp = cpu.ebp;
    const x86::reg32 name = cpu.eax;
    const x86::reg32 path = esp - 0x14 - 0x200;  // below the five registers it keeps: its buffer
    cpu.esp = path;
    if (!call(app, cpu, at(app, kLoadLibraryA), {name}))
        return;
    put(app, kRendererModule, cpu.eax);
    if (cpu.eax == 0)
    {
        if (!call(app, cpu, kSprintf, {path, 0x54d3a0 /* "%sa" */, name})
            || !call(app, cpu, at(app, kLoadLibraryA), {path}))
            return;
        put(app, kRendererModule, cpu.eax);
    }
    x86::reg32 opened = 0;
    if (at(app, kRendererModule) != 0)
    {
        cpu.esp = path - 4;  // below edi, which it keeps while it looks
        for (const auto& function : kThrashFunctions)
        {
            if (!call(app, cpu, at(app, kGetProcAddress), {at(app, kRendererModule), function.name}))
                return;
            put(app, function.slot, cpu.eax);
        }
        cpu.esp = path;
        opened = 1;
    }
    put(app, kRendererIs3dfx, 0);
    if (opened)
    {
        if (const x86::reg32 setState = at(app, kThrashSetState);
            setState != 0 && !call(app, cpu, setState, {0x1f, kThrashVersion}))
            return;
        if (const x86::reg32 about = at(app, kThrashAbout))
        {
            if (!call(app, cpu, about))
                return;
            if (const x86::reg32 record = cpu.eax)
            {
                if (at(app, record) == 0x33444658)  // "XFD3"
                    put(app, kRendererIs3dfx, 1);
                const x86::reg32 version = at(app, record + 8);
                if (version != kThrashVersion)
                {
                    // "THRASH_opendll - DRIVER VERSION MISMATCH. LIBRARY WANTS 104. YOUR MODULE IS %d..."
                    if (!call(app, cpu, Message, {0x54d67c, version}))
                        return;
                    opened = 0;
                }
            }
        }
    }
    cpu.eax = opened;
    cpu.ebx = ebx;
    cpu.ecx = ecx;
    cpu.edx = edx;
    cpu.esi = esi;
    cpu.ebp = ebp;
    cpu.esp = esp + 4;
}

/* The game's fill (0x4e0721; 0x4e0716 spreads bl over ebx first, 0x4e070c
 * fills with zeros), EA's, not the runtime's memset: edx bytes at eax from the pattern in ebx --
 * up to an 8-byte boundary a byte, a word and a dword at a time, then 32 and 8
 * bytes at a time (as doubles when its two low bytes are one value other than
 * 0xff), then what is left.  eax ends past the bytes -- but for a last odd
 * one, which it ends on; edx and the flags as its last steps leave them. */
namespace
{

/* What fillCore leaves of the flags: cmp, add and test's. */
struct IntegerFlags
{
    bool cf = false;
    bool of = false;
    bool zf = false;
    bool sf = false;

    void logic(x86::reg32 value)
    {
        cf = of = false;
        zf = !value;
        sf = value >> 31;
    }
    void compare(x86::reg32 a, x86::reg32 b)
    {
        const x86::reg32 result = a - b;
        cf = a < b;
        of = ((a >> 31) != (result >> 31)) && ((a >> 31) != (b >> 31));
        zf = !result;
        sf = result >> 31;
    }
    void store(x86::CPU& cpu) const
    {
        cpu.flags.cf = cf;
        cpu.flags.of = of;
        cpu.flags.zf = zf;
        cpu.flags.sf = sf;
    }
};

void fillCore(win32::WinApplication* app, x86::CPU& cpu)
{
    x86::reg32 at = cpu.eax;
    x86::sreg32 left = x86::sreg32(cpu.edx);
    const x86::reg32 pattern = cpu.ebx;
    x86::reg8* memory = &app->getMemory<x86::reg8>(0);
    auto put = [&](x86::reg32 bytes) {
        std::memcpy(memory + at, &pattern, bytes);
        at += bytes;
    };
    IntegerFlags flags;
    if (at & 7)
    {
        if ((at & 1) && left >= 1)
        {
            put(1);
            left -= 1;
        }
        if ((at & 2) && left >= 2)
        {
            put(2);
            left -= 2;
        }
        if ((at & 4) && left >= 4)
        {
            put(4);
            left -= 4;
        }
    }
    flags.compare(x86::reg32(left), 0);  // cmp edx, 0
    if (left >= 0)
    {
        // Eight bytes at a time: the pattern twice, as the dwords store it --
        // or, bl and bh alike and not 0xff, as a double the x87 loads from
        // the pattern pushed twice and stores (fld qword, fst qword), which
        // makes a signalling NaN of it quiet.
        x86::reg8 unit[8];
        std::memcpy(unit, &pattern, 4);
        std::memcpy(unit + 4, &pattern, 4);
        const bool x87 = (pattern & 0xff) != 0xff && (pattern & 0xff) == (pattern >> 8 & 0xff);
        if (x87)
        {
            double loaded;
            std::memcpy(&loaded, unit, 8);
            cpu.fpu.push(x86::Float(loaded));
            const double stored = double(cpu.fpu.st(0));
            std::memcpy(unit, &stored, 8);
        }
        // 32 at a time: sub edx, 0x20 until it goes negative; then 8 at a time
        left -= 0x20;
        if (left >= 0)
        {
            // one fill for the whole run of 32-byte blocks
            const x86::reg32 blocks = x86::reg32(left) / 0x20 + 1;
            x86::reg8* to = memory + at;
            for (x86::reg32 i = 0; i < blocks * 4; ++i)
                std::memcpy(to + i * 8, unit, 8);
            at += blocks * 0x20;
            left -= x86::sreg32(blocks * 0x20);
        }
        left += 0x18;
        while (left >= 0)
        {
            std::memcpy(memory + at, unit, 8);
            at += 8;
            left -= 8;
        }
        if (x87)
            cpu.fpu.count -= 1;  // fstp st(0)
        const x86::reg32 before = x86::reg32(left);
        left += 8;
        // add edx, 8
        const x86::reg32 sum = x86::reg32(left);
        flags.cf = sum < before;
        flags.of = (~(before ^ 8u) & (before ^ sum)) >> 31;
        flags.zf = sum == 0;
        flags.sf = sum >> 31;
        if (left != 0)
        {
            if (left & 4)
                put(4);
            if (left & 2)
                put(2);
            if (left & 1)
                memory[at] = x86::reg8(pattern);  // the last byte: eax not moved past it
            flags.logic(x86::reg32(left) & 1);  // test edx, 1
        }
    }
    flags.store(cpu);
    cpu.eax = at;
    cpu.edx = x86::reg32(left);
}

}  // namespace

/* 0x4e0721: edx bytes at eax of the pattern in ebx. */
void fillWord(win32::WinApplication* app, x86::CPU& cpu)
{
    fillCore(app, cpu);
    cpu.esp += 4;
}

/* 0x4e0716: edx bytes at eax of bl. */
void fillByte(win32::WinApplication* app, x86::CPU& cpu)
{
    x86::reg32 ebx = (cpu.ebx & 0xffff0000) | (cpu.ebx & 0xff) << 8 | (cpu.ebx & 0xff);
    ebx |= ebx << 16;
    cpu.ebx = ebx;
    fillCore(app, cpu);
    cpu.esp += 4;
}

/* 0x4e070c: edx bytes at eax cleared. */
void fillZero(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 ebx = cpu.ebx;
    cpu.ebx = 0;
    fillCore(app, cpu);
    cpu.ebx = ebx;
    cpu.esp += 4;
}


/* The game asked the processor what it could do and timed it, to choose what to
 * run on it.  No phone needs that asked: what is left of the answers is what the
 * sound's set-up and the memmove read, MMX's routines (native_mix.cpp). */
namespace removed
{

void processorChecked(win32::WinApplication* app, x86::CPU& cpu)
{
    app->getMemory<x86::reg32>(0x5643b4) = 1;  // MMX's set of the mixing's routines
    app->getMemory<x86::reg32>(0x5643ac) = 1;
    app->getMemory<x86::reg32>(0x5643bc) = 0;  // no timestamp counter to read
    cpu.esp += 4;
}

void processorSpeed(win32::WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    cpu.esp += 4;
}

}  // namespace removed

}  // namespace nfs3hp
