#include "native_game.h"
#include "native_ea_support.h"
#include "native_ea_threads.h"
#include "native_watcom.h"
#include "port_snd.h"
#include "port_snd_mixer.h"
#include "native_windows.h"
#include "isnd_calls.h"
#include "thrash_calls.h"
#include "host_threads.h"
#include "host_window.h"
#include "host_system.h"
#include <cstring>
#include <iterator>

namespace nfs3hp::port
{
// port_snd_voices.cpp's (port_snd.h): the sound's tick with the registers it reads as values.
Result tickSoundFrom(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 esp, x86::reg32 eax, x86::reg32 edx,
                     x86::reg32 ecx, x86::reg32 ebp);
}

/* nfs3.exe's own functions the port has natively, which
 * the game's code calls by name: the game's side
 * of its two driver interfaces, THRASH and iSND, the logger they report
 * through, the functions of the game's that iSNDloaddll hands eacsnd, and the
 * fill the game's code clears and fills its memory with.  The names are EA's
 * where the game's own messages give them.
 *
 * Each keeps what its caller counts on: the game's own callers are Watcom
 * code, which counts on every register it handed no argument in (and the
 * functions here keep even those, but eax, as the game's code did); eacsnd
 * calls the lambdas stdcall or cdecl, and counts on ebx, esi, edi and ebp.  They call what the
 * game's code called as it did -- the arguments where it pushed them, in the
 * registers it had them in, and the stack where it had it, which is where a
 * pointer into its frame points.  The game's own functions they call are
 * called by address. */

namespace nfs3hp
{

void commMessageTrace(win32::WinApplication* app, x86::CPU& cpu);  // native_vertices.cpp

x86::reg32 Error(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 Message(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 AcquireWindow(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 iSNDloaddll(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 SNDcaps(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);

namespace
{

using guest::at;
using guest::put;
using guest::windows;

using guest::EspKept;

typedef x86::reg32 (*Thunk)(win32::WinApplication*, x86::CPU&, x86::Args&);

/* A call of one of this file's thunks as the game's code makes one (guest::through's layout): `arguments`
 * pushed below esp, `more` after them, the return address's slot below them all, esp back where it was
 * after it.  Its eax; nullopt when the game stopped. */
std::optional<x86::reg32> thunkCall(win32::WinApplication* app, x86::CPU& cpu, Thunk target, x86::Args& registers,
                                    std::initializer_list<x86::reg32> arguments, const guest::Words& more = guest::Words({}))
{
    const x86::reg32 esp = cpu.esp;
    x86::reg32 slot = esp - 4 * x86::reg32(arguments.size() + more.count);
    cpu.esp = slot - 4;
    for (const x86::reg32 argument : arguments)
    {
        put(app, slot, argument);
        slot += 4;
    }
    for (unsigned i = 0; i < more.count; ++i)
    {
        put(app, slot, more.word[i]);
        slot += 4;
    }
    const x86::reg32 eax = target(app, cpu, registers);
    if (cpu.terminate)
        return std::nullopt;
    cpu.esp = esp;
    return eax;
}

/* The sound's export at Slot with `words`, every register but eax kept as the handlers keep them.  The
 * eax soundFrom is given goes only to a function another than the port's the game put in the slot,
 * which it never does (the callee's typed function takes only its words). */
template <x86::reg32 Slot, auto Fn, typename... W>
std::optional<x86::reg32> soundKept(win32::WinApplication* app, x86::CPU& cpu, W... words)
{
    const EspKept kept(cpu);
    return guest::soundFrom<Slot, Fn>(app, cpu, cpu.esp, 0u, words...);
}

/* The game's imports (their slots), as its code calls them: the port's own Windows functions,
 * called as C++ (native_windows.h). */
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
constexpr x86::reg32 kFflush = 0x4f7d70;    // (stream)

/* Where an Error comes from, for its text: each holds a file's name, a
 * function's and a line, in that order, set before the call. */
constexpr x86::reg32 kWhere = 0x552190;
constexpr x86::reg32 kCalledFrom = 0x55219c;  // what called that, where somebody says

constexpr x86::reg32 kWindow = 0x9f3d24;  // the game's window

}  // namespace

/* AcquireWindow's core (0x4eba70): the game's window. */
x86::reg32 ea::acquireWindow(win32::WinApplication* app)
{
    return at(app, kWindow);
}

/* AcquireWindow (0x4eba70): the game's window. */
x86::reg32 AcquireWindow(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = ea::acquireWindow(app);
    cpu.esp += 4;
    return args.eax;
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

/* `format` with the file, line and function at `where` into `buffer`, then
 * on the end of Error's text, as Error's code does it -- the copy inline. */
bool addWhere(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buffer, x86::reg32 where, x86::reg32 format)
{
    guest::direct<0x4df690, watcom::clib::sprintf>(app, cpu, buffer, format,
                                                   guest::Words({at(app, where), at(app, where + 8), at(app, where + 4)}));
    if (cpu.terminate)
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
x86::reg32 Error(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    commMessageTrace(app, cpu);
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 ebx = args.ebx, ecx = args.ecx, edx = args.edx, esi = args.esi, edi = args.edi, ebp = args.ebp;
    // A pointer's callee (the game's hooks) handed every register as Error has it now.
    const auto viaPointer = [&](x86::reg32 target) {
        x86::Args registers = args;
        return guest::pointerCall(app, cpu, target, registers);
    };
    const auto back = [&]() {
        args.ebx = ebx;
        args.ecx = ecx;
        args.edx = edx;
        args.esi = esi;
        args.edi = edi;
        args.ebp = ebp;
        cpu.esp = esp + 4;
    };
    if (at(app, kErrorUnderWay) != 0)
    {
        back();
        return args.eax;
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
        // Its arguments in eax, edx and ebx, as the call by address loads them.
        args.eax = kErrorText;
        args.edx = format;
        args.ebx = arguments;
        guest::direct<0x4df674, watcom::clib::vsprintf>(app, cpu, kErrorText, format, arguments);
        if (cpu.terminate)
            return args.eax;
    }
    else
    {
        app->getMemory<x86::reg8>(kErrorText) = 0;
    }
    if (at(app, kDebugOutput) != 0)
    {
        if (!windows<kOutputDebugStringA, &host::debugString>(app, cpu, kErrorText))
            return args.eax;
        if (at(app, kDebugStop) != 0 && !windows<kDebugBreak, &host::debugBreak>(app, cpu))
            return args.eax;
    }
    if (const x86::reg32 filter = at(app, kErrorFilter))
    {
        args.eax = kErrorText;
        const std::optional<x86::reg32> filtered = viaPointer(filter);
        if (!filtered)
            return args.eax;
        args.eax = *filtered;
        if (args.eax == 0)
        {
            put(app, kErrorUnderWay, 0);
            back();
            return args.eax;
        }
    }
    if (at(app, kErrorWhere) != 0)
    {
        if (at(app, kWhere) != 0 && !addWhere(app, cpu, buffer, kWhere, kWhereFormat))
            return args.eax;
        if (at(app, kCalledFrom) != 0 && !addWhere(app, cpu, buffer, kCalledFrom, kCalledFromFormat))
            return args.eax;
    }
    x86::reg32 lock = 0;
    if (const x86::reg32 before = at(app, kErrorLock))
    {
        const std::optional<x86::reg32> locked = viaPointer(before);
        if (!locked)
            return args.eax;
        args.eax = lock = *locked;
    }
    if (const x86::reg32 before = at(app, kErrorBefore); before != 0)
    {
        const std::optional<x86::reg32> readied = viaPointer(before);
        if (!readied)
            return args.eax;
        args.eax = *readied;
    }
    const auto cursor = windows<kShowCursor, &host::showCursor>(app, cpu, 1);
    if (!cursor)
        return args.eax;
    args.eax = *cursor;
    // MB_OKCANCEL | MB_ICONEXCLAMATION | MB_SYSTEMMODAL | MB_SETFOREGROUND
    const auto boxed = windows<kMessageBoxA, &host::messageBox>(app, cpu, 0, kErrorText, kErrorCaption, 0x11031);
    if (!boxed)
        return args.eax;
    args.eax = *boxed;
    if (args.eax == 1 && at(app, kErrorPrintOk) != 0 && at(app, kErrorOk) != 0)
    {
        guest::direct<0x4df6c0, ea::message>(app, cpu, kJustText, guest::Words({kErrorText}));
        if (cpu.terminate)
            return args.eax;
        const std::optional<x86::reg32> okayed = viaPointer(at(app, kErrorOk));
        if (!okayed)
            return args.eax;
        args.eax = *okayed;
    }
    if (lock != 0 && !windows<kLeaveCriticalSection, &host::leaveLock>(app, cpu, lock))
        return args.eax;
    const std::optional<bool> onMainThread = guest::direct<0x4df930, ea::isThreadNow>(app, cpu, 0u);
    if (!onMainThread)
        return args.eax;
    args.eax = *onMainThread ? 1 : 0;
    if (*onMainThread)
    {
        args.eax = 0;
        guest::direct<0x4dfd04, watcom::clib::exit>(app, cpu, 0u);
        if (cpu.terminate)
            return args.eax;
    }
    else if (!windows<kExitProcess, &host::exitProcess>(app, cpu, args.eax))
    {
        return args.eax;
    }
    back();
    return args.eax;
}

/* Message (0x4df6c0): cdecl (format, ...), printf -- vfprintf to stdout;
 * keeps ebx and edx, which vfprintf does not. */
void ea::message(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 format, guest::Words words)
{
    // Called as C++ (guest::direct), the return address's slot already under esp: the words pushed
    // where the game's call pushes them, over that slot, so Message's frame lies where its code has it.
    cpu.esp += 4;
    x86::Args registers;
    const bool returned = thunkCall(app, cpu, &Message, registers, {format}, words).has_value();
    if (returned)
        cpu.esp -= 4;
}

x86::reg32 Message(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 ebx = args.ebx, edx = args.edx;
    const x86::reg32 arguments = esp - 0xc;  // its va_list, below the two registers it keeps
    put(app, arguments, esp + 8);
    cpu.esp = arguments;
    args.eax = guest::direct<0x4ff014, watcom::clib::vfprintf>(app, cpu, kStdout, at(app, esp + 4), arguments);
    if (cpu.terminate)
        return args.eax;
    args.ebx = ebx;
    args.edx = edx;
    cpu.esp = esp + 4;
    return args.eax;
}

/* ---- iSND, the game's side: eacsnd loaded and called ---- */

namespace
{

constexpr x86::reg32 kSoundModule = 0x9ef600;   // eacsnd.dll's handle
constexpr x86::reg32 kSoundOptions = 0x9ef608;  // what the sound is started with
/* iSNDloaddll's lookups after setfunctions, into 0x9ef61c.. in this order
 * (isnd.h, kDirect*), called through them: guest::sound (isnd_calls.h). */
constexpr x86::reg32 kSoundFunctions = 0x9ef61c;
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
    guest::Words rest({});
    for (const x86::reg32* word = arguments.begin() + 1; word < arguments.end(); ++word)
        rest.word[rest.count++] = *word;
    guest::direct<0x401010, ea::error>(app, cpu, *arguments.begin(), rest);
    return !cpu.terminate;
}

/* iSNDloaddll's work: false when the game stopped, else `result` 0, -1 for
 * what it could not find, -11 for the wrong version. */
bool loadSound(win32::WinApplication* app, x86::CPU& cpu, x86::reg32& result)
{
    result = 0xffffffff;
    const auto loaded = windows<kLoadLibraryA, &host::loadLibrary>(app, cpu, kSoundDll);
    if (!loaded)
        return false;
    const x86::reg32 module = *loaded;
    put(app, kSoundModule, module);
    if (module == 0)  // "COULDN'T LOAD/FIND EACSND.DLL..."
        return soundError(app, cpu, kLoadName, 0x153, {0x549c78});
    const auto versionFound = windows<kGetProcAddress, &host::procAddress>(app, cpu, module, kVersionName);
    if (!versionFound)
        return false;
    const x86::reg32 version = *versionFound;
    if (version == 0)  // "COULDN'T FIND iSNDdllversion."
        return soundError(app, cpu, kLoadName, 0x15f, {0x549cec});
    x86::Args versionRegisters;
    const std::optional<x86::reg32> found = guest::pointerCall(app, cpu, version, versionRegisters);
    if (!found)
        return false;
    if (*found != kSoundVersion)
    {
        // "EACSND.DLL VERSION MISMATCH. REQUIRED %i.%i, FOUND %i.%i."
        result = 0xfffffff5;
        return soundError(app, cpu, kLoadName, 0x169,
                          {0x549d1c, 0x10, 3, x86::reg32(x86::sreg32(*found) >> 16), *found & 0xffff});
    }
    const auto setFound = windows<kGetProcAddress, &host::procAddress>(app, cpu, at(app, kSoundModule), kSetFunctionsName);
    if (!setFound)
        return false;
    const x86::reg32 setFunctions = *setFound;
    if (setFunctions == 0)  // "COULDN'T FIND setfunctions."
        return soundError(app, cpu, kLoadName, 0x176, {0x549d84});
    const x86::reg32* l = kSoundLambdas;
    x86::Args setRegisters;
    if (!guest::pointerCall(app, cpu, setFunctions, setRegisters, {l[0], l[1], l[2], l[3], l[4], l[5], l[6], l[7]}))
        return false;
    x86::reg32 slot = kSoundFunctions;
    for (const x86::reg32 name : kSoundFunctionNames)
    {
        const auto found = windows<kGetProcAddress, &host::procAddress>(app, cpu, at(app, kSoundModule), name);
        if (!found)
            return false;
        put(app, slot, *found);
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
x86::reg32 iSNDloaddll(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 ebx = args.ebx, ecx = args.ecx, edx = args.edx, esi = args.esi, edi = args.edi, ebp = args.ebp;
    x86::reg32 result = 0;
    if (at(app, kSoundModule) == 0)
    {
        cpu.esp = esp - 0x18;  // below the six registers it keeps
        if (!loadSound(app, cpu, result))
            return args.eax;
    }
    args.eax = result;
    args.ebx = ebx;
    args.ecx = ecx;
    args.edx = edx;
    args.esi = esi;
    args.edi = edi;
    args.ebp = ebp;
    cpu.esp = esp + 4;
    return args.eax;
}

/* SNDcaps (0x4e21b0): what the card can, as iSNDdirectcaps says for the
 * game's window, 0x10000 added if that is above 0; eacsnd loaded first, and
 * the tick the game's own unless it set another.  Keeps every register but
 * eax. */
x86::reg32 SNDcaps(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 ecx = args.ecx, edx = args.edx;
    if (at(app, kSoundTick) == 0)
        put(app, kSoundTick, kGameTick);
    cpu.esp = esp - 8;  // below the two registers it keeps
    x86::Args loadRegisters = args;
    if (!thunkCall(app, cpu, &iSNDloaddll, loadRegisters, {}))
        return args.eax;
    const x86::reg32 window = guest::direct<0x4eba70, ea::acquireWindow>(app, cpu);
    args.eax = window;  // as the call leaves it: the sound's export is handed it
    if (window == 0)
    {
        cpu.esp = esp - 0x14;  // below ebx, esi and edi too, which it keeps around the call
        // "SNDcaps - MAIN WINDOW HANDLE IS INVALID."
        if (!soundError(app, cpu, kCapsName, 0x22d, {0x549f28}))
            return args.eax;
        cpu.esp = esp - 8;
    }
    const std::optional<x86::reg32> caps = soundKept<isnd::kDirectCaps, &port::eacsnd::caps>(app, cpu, window);
    if (!caps)
        return args.eax;
    args.eax = x86::sreg32(*caps) > 0 ? *caps | 0x10000 : *caps;
    args.ecx = ecx;
    args.edx = edx;
    cpu.esp = esp + 4;
    return args.eax;
}

/* Is3DSoundAvailable (0x40faf0): 1 if SNDcaps has 0x20000, else 0.  (It hands
 * SNDcaps -1 in eax, which SNDcaps never reads.) */
x86::reg32 Is3DSoundAvailable(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 esp = cpu.esp;
    cpu.esp = esp - 4;  // below ebp, which it keeps
    x86::Args capsRegisters = args;
    const std::optional<x86::reg32> caps = thunkCall(app, cpu, &SNDcaps, capsRegisters, {});
    if (!caps)
        return args.eax;
    args.eax = (*caps & 0x20000) != 0 ? 1 : 0;
    cpu.esp = esp + 4;
    return args.eax;
}

/* audiofocushandler (0x4e1d48): eax nonzero, the game has the focus and the
 * sound is started for its window with kSoundOptions; zero, it is stopped.
 * Under the sound's lock; keeps every register but eax. */
x86::reg32 audiofocushandler(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 ebx = args.ebx, ecx = args.ecx, edx = args.edx, ebp = args.ebp;
    const x86::reg32 focus = args.eax;
    cpu.esp = esp - 0x10;  // below the four registers it keeps
    if (!windows<kEnterCriticalSection, &host::enterLock>(app, cpu, at(app, kSoundLock)))
        return args.eax;
    if (focus != 0)
    {
        const x86::reg32 window = guest::direct<0x4eba70, ea::acquireWindow>(app, cpu);
        args.eax = window;  // as the call leaves it
        if (window == 0)
        {
            cpu.esp = esp - 0x18;  // below edi and esi too, which it keeps around the call
            // "audiofocushandler - MAIN WINDOW HANDLE IS INVALID."
            if (!soundError(app, cpu, kFocusName, 0x124, {0x549c2c}))
                return args.eax;
            cpu.esp = esp - 0x10;
        }
        if (!soundKept<isnd::kDirectStart, &port::eacsnd::start>(app, cpu, at(app, kSoundOptions), window))
            return args.eax;
    }
    else if (!soundKept<isnd::kDirectStop, &port::eacsnd::stop>(app, cpu))
    {
        return args.eax;
    }
    if (!windows<kLeaveCriticalSection, &host::leaveLock>(app, cpu, at(app, kSoundLock)))
        return args.eax;
    args.ebx = ebx;
    args.ecx = ecx;
    args.edx = edx;
    args.ebp = ebp;
    cpu.esp = esp + 4;
    return args.eax;
}

/* SoundServeHandler (0x4e2234): iSNDdirectserve under the sound's lock;
 * keeps every register but eax. */
x86::reg32 SoundServeHandler(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 ecx = args.ecx, edx = args.edx;
    cpu.esp = esp - 8;  // below the two registers it keeps
    if (!windows<kEnterCriticalSection, &host::enterLock>(app, cpu, at(app, kSoundLock))
        || !soundKept<isnd::kDirectServe, &port::eacsnd::serve>(app, cpu)
        || !windows<kLeaveCriticalSection, &host::leaveLock>(app, cpu, at(app, kSoundLock)))
        return args.eax;
    args.ecx = ecx;
    args.edx = edx;
    cpu.esp = esp + 4;
    return args.eax;
}

/* The lambdas iSNDloaddll hands eacsnd (kSoundLambdas), each to the game's
 * own function behind it: stdcall, and the three that format, cdecl.  The
 * four the port's eacsnd calls are C++ as well (isnd_calls.h), their natives
 * thin wrappers: the words off the stack and popped, the registers as the C++
 * leaves them -- eax and edx the game's function's arguments, as its code
 * leaves them. */

/* AcquireSoundDataLambda's (0x4e1a24): (buffer, frames), the mix. */
std::optional<x86::reg32> acquireSoundDataOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buffer,
                                             x86::reg32 frames)
{
    // 0x500864, the mix: every register but eax kept, as its entry keeps them.
    const EspKept kept(cpu);
    const x86::reg32 mixed = port::mixFrames(app, cpu, cpu.esp - 4, buffer, x86::sreg32(frames), buffer);
    if (cpu.terminate)
        return std::nullopt;
    return mixed;
}

x86::reg32 AcquireSoundDataLambda(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 esp = cpu.esp;
    if (!acquireSoundDataOf(app, cpu, at(app, esp + 4), at(app, esp + 8)))
        return args.eax;
    cpu.esp = esp + 4 + 8;
    return args.eax;
}

/* StopSoundBufferLambda's (0x4e1a00): (buffer). */
std::optional<x86::reg32> stopSoundBufferOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buffer)
{
    // 0x4ffb64, iSNDfreechan: every register but eax kept, as its entry keeps them.
    const EspKept kept(cpu);
    const port::Result freed = port::freeVoice(app, cpu, cpu.esp - 4, x86::sreg32(buffer));
    if (!freed || cpu.terminate)
        return std::nullopt;
    return x86::reg32(*freed);
}

x86::reg32 StopSoundBufferLambda(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 esp = cpu.esp;
    if (!stopSoundBufferOf(app, cpu, at(app, esp + 4)))
        return args.eax;
    cpu.esp = esp + 4 + 4;
    return args.eax;
}

/* AcquireSoundBufferPositionLambda's (0x4e1a0c): (three words, the last
 * where the position goes); keeps ebx. */
std::optional<x86::reg32> acquireSoundBufferPositionOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 first,
                                                       x86::reg32 second, x86::reg32 position)
{
    // 0x4ffce0: ebx kept, below it the call's return address's slot.
    const EspKept kept(cpu);
    const port::Result placed = port::bufferPositioned(app, cpu, cpu.esp - 8, first, second, position);
    if (!placed || cpu.terminate)
        return std::nullopt;
    return x86::reg32(*placed);
}

x86::reg32 AcquireSoundBufferPositionLambda(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 esp = cpu.esp;
    if (!acquireSoundBufferPositionOf(app, cpu, at(app, esp + 4), at(app, esp + 8), at(app, esp + 0xc)))
        return args.eax;
    cpu.esp = esp + 4 + 0xc;
    return args.eax;
}

/* TickSoundLambda's (0x4e1adc): the game's 100 Hz tick (kSoundTick).  The
 * library's own, port::soundTick (0x500c80, kGameTick, which SNDcaps puts
 * there), as its C++, port::tickSound: its frame where the call's return
 * address would be, every register but eax kept as its thunk keeps them.
 * Any other function the game set goes through the pointer. */
std::optional<x86::reg32> tickSoundWith(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 eax, x86::reg32 edx,
                                      x86::reg32 ecx, x86::reg32 ebp)
{
    const x86::reg32 tick = at(app, kSoundTick);
#ifndef NFS3HP_SOUND_THROUGH_SLOTS
    if (tick == kGameTick)  // the library's own: port::soundTick is registered there
    {
        const EspKept kept(cpu);
        const port::Result result = port::tickSoundFrom(app, cpu, cpu.esp - 4, eax, edx, ecx, ebp);
        if (!result)
            return std::nullopt;
        return x86::reg32(*result);
    }
#endif
    x86::Args registers;
    registers.eax = eax;
    registers.edx = edx;
    registers.ecx = ecx;
    registers.ebp = ebp;
    return guest::pointerCall(app, cpu, tick, registers);
}

/* The tick as guest::sound calls it (native_sound.cpp): the registers it is handed as the caller left them. */
std::optional<x86::reg32> tickSoundOf(win32::WinApplication* app, x86::CPU& cpu)
{
    return tickSoundWith(app, cpu, 0, 0, 0, 0);
}

x86::reg32 TickSoundLambda(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 esp = cpu.esp;
    const std::optional<x86::reg32> ticked = tickSoundWith(app, cpu, args.eax, args.edx, args.ecx, args.ebp);
    if (!ticked)
        return args.eax;
    args.eax = *ticked;
    cpu.esp = esp + 4;
    return args.eax;
}

/* PrintSoundLambda (0x4e1a34): cdecl (format, ...), to stdout, flushed --
 * Message handed the format and the address of a va_list of the rest, as if
 * it were the first of them. */
x86::reg32 PrintSoundLambda(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 arguments = esp - 4;
    put(app, arguments, esp + 8);
    cpu.esp = arguments;
    guest::direct<0x4df6c0, ea::message>(app, cpu, at(app, esp + 4), guest::Words({arguments}));
    if (cpu.terminate)
        return args.eax;
    args.eax = guest::direct<0x4f7d70, watcom::clib::fflush>(app, cpu, kStdout);
    if (cpu.terminate)
        return args.eax;
    cpu.esp = esp + 4;
    return args.eax;
}

/* TraceSoundLambda (0x4e1a64): cdecl (format, ...), into the game's log as
 * PrintSoundLambda hands Message its arguments. */
x86::reg32 TraceSoundLambda(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 arguments = esp - 4;
    put(app, arguments, esp + 8);
    cpu.esp = arguments;
    const std::optional<x86::reg32> traced =
        guest::direct<0x4e08f0, ea::monoPrinted>(app, cpu, args.eax, at(app, esp + 4), guest::Words({arguments}));
    if (!traced || cpu.terminate)
        return args.eax;
    args.eax = *traced;
    cpu.esp = esp + 4;
    return args.eax;
}

/* abortmessagewrapper (0x4e1a88): cdecl (text, ...), stdout flushed and the
 * text an Error from this function -- with the address of a va_list of the
 * rest as its one argument. */
x86::reg32 abortmessagewrapper(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 arguments = esp - 0x10;  // its va_list, below ebx, esi and edi, which it keeps
    cpu.esp = arguments;
    args.eax = guest::direct<0x4f7d70, watcom::clib::fflush>(app, cpu, kStdout);
    if (cpu.terminate)
        return args.eax;
    put(app, arguments, esp + 8);
    if (!soundError(app, cpu, kAbortmessagewrapperName, 0x92, {at(app, esp + 4), arguments}))
        return args.eax;
    cpu.esp = esp + 4;
    return args.eax;
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
x86::reg32 iSNDconvertsample(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 esp = cpu.esp;
    const auto argument = [&](x86::reg32 i) { return at(app, esp + 4 * i); };
    const x86::reg32 source = argument(1), representation = argument(2), channels = argument(3);
    const x86::reg32 frames = argument(6);
    const x86::reg32 frame = esp - 0x40;  // below the four registers it keeps, and its 0x30 bytes
    cpu.esp = frame;
    // "CAN'T CONVERT XA WITH %i CHANNELS."
    if (x86::sreg32(channels) > 2 && !soundError(app, cpu, kConvertName, 0xaa, {0x549b48, channels}))
        return args.eax;
    // "CAN'T HANDLE dstnumchan %i"
    if (argument(9) != 1 && !soundError(app, cpu, kConvertName, 0xb0, {0x549b80, argument(9)}))
        return args.eax;
    const auto allocated = windows<kVirtualAlloc, &host::virtualAlloc>(app, cpu, 0, frames * 8 + 0x4e20, 0x3000, 4);
    if (!allocated)
        return args.eax;
    args.eax = *allocated;
    x86::reg32 buffer = args.eax;
    switch (representation)
    {
    case 0:
    case 2:
        if (representation == 0)
            guest::direct<0x5008a0, port::samples16Converted>(app, cpu, std::int32_t(frames * channels), source, buffer);
        else
            guest::direct<0x500b10, port::samples8Converted>(app, cpu, std::int32_t(frames * channels), source, buffer);
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
            guest::direct<0x500b90, port::xaMonoConverted>(app, cpu, record);
        }
        else if (channels == 2)
        {
            const x86::reg32 record = frame;
            put(app, record, frames);
            for (x86::reg32 offset = 4; offset < 0x14; offset += 4)
                put(app, record + offset, 0);
            put(app, record + 0x14, source);
            put(app, record + 0x18, buffer);
            guest::direct<0x500990, port::xaStereoConverted>(app, cpu, record);
        }
        break;
    default:
        // "CAN'T HANDLE srcsamplerep %i"
        if (!soundError(app, cpu, kConvertName, 0xe0, {0x549bb0, representation}))
            return args.eax;
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
            return args.eax;
    }
    else
    {
        guest::direct<0x500910, port::floatsPacked>(app, cpu, count, buffer, argument(7));
    }
    if (!windows<kVirtualFree, &host::virtualFree>(app, cpu, buffer, 0, 0x8000))  // MEM_RELEASE
        return args.eax;
    args.eax = 0;
    cpu.esp = esp + 4 + 0x24;
    return args.eax;
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

}  // namespace

/* THRASH_opendll (0x4f8390): eax the renderer's name -- "voodoo2" -- and that
 * with an "a" after it if that will not load; its 34 functions looked up,
 * state 0x1f set to 104, and THRASH_about asked whether it is 3dfx's and of
 * version 104.  1 if it loaded and is (or will not say), else 0, after a
 * Message when it is not.  Keeps every register but eax. */
std::optional<x86::reg32> ea::thrashOpened(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 path = esp - 0x14 - 0x200;  // below the five registers it keeps: its buffer
    cpu.esp = path;
    auto module = windows<kLoadLibraryA, &host::loadLibrary>(app, cpu, name);
    if (!module)
        return std::nullopt;
    put(app, kRendererModule, *module);
    if (*module == 0)
    {
        guest::direct<0x4df690, watcom::clib::sprintf>(app, cpu, path, 0x54d3a0u /* "%sa" */, guest::Words({name}));
        if (cpu.terminate || !(module = windows<kLoadLibraryA, &host::loadLibrary>(app, cpu, path)))
            return std::nullopt;
        put(app, kRendererModule, *module);
    }
    x86::reg32 opened = 0;
    if (at(app, kRendererModule) != 0)
    {
        cpu.esp = path - 4;  // below edi, which it keeps while it looks
        for (const auto& function : kThrashFunctions)
        {
            const auto found = windows<kGetProcAddress, &host::procAddress>(app, cpu, at(app, kRendererModule), function.name);
            if (!found)
                return std::nullopt;
            put(app, function.slot, *found);
        }
        cpu.esp = path;
        opened = 1;
    }
    put(app, kRendererIs3dfx, 0);
    if (opened)
    {
        if (at(app, thrash::kThrashSetState) != 0
            && !guest::driver<thrash::kThrashSetState, &port::driver::setState>(app, cpu, 0x1f, kThrashVersion))
            return std::nullopt;
        if (at(app, thrash::kThrashAbout) != 0)
        {
            const std::optional<x86::reg32> about = guest::driver<thrash::kThrashAbout, &port::driver::about>(app, cpu);
            if (!about)
                return std::nullopt;
            if (const x86::reg32 record = *about)
            {
                if (at(app, record) == 0x33444658)  // "XFD3"
                    put(app, kRendererIs3dfx, 1);
                const x86::reg32 version = at(app, record + 8);
                if (version != kThrashVersion)
                {
                    // "THRASH_opendll - DRIVER VERSION MISMATCH. LIBRARY WANTS 104. YOUR MODULE IS %d..."
                    x86::Args registers;
                    if (!thunkCall(app, cpu, &Message, registers, {0x54d67c, version}))
                        return std::nullopt;
                    opened = 0;
                }
            }
        }
    }
    cpu.esp = esp;
    return opened;
}

x86::reg32 THRASH_opendll(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 ebx = args.ebx, ecx = args.ecx, edx = args.edx, esi = args.esi, ebp = args.ebp;
    const auto opened = ea::thrashOpened(app, cpu, args.eax);
    if (!opened)
        return args.eax;
    args.eax = *opened;
    args.ebx = ebx;
    args.ecx = ecx;
    args.edx = edx;
    args.esi = esi;
    args.ebp = ebp;
    cpu.esp = esp + 4;
    return args.eax;
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
};

/* What fillCore leaves: eax, edx and the flags. */
struct FillResult
{
    x86::reg32 at;
    x86::reg32 left;
    IntegerFlags flags;
};

/* The fill itself; `write` false only works out where it would end (the eax). */
FillResult fillRun(x86::reg8* memory, x86::reg32 at, x86::sreg32 left, x86::reg32 pattern, bool write)
{
    auto put = [&](x86::reg32 bytes) {
        if (write)
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
            // fld qword, and each fst qword stores the double it holds: a local here.
            double loaded;
            std::memcpy(&loaded, unit, 8);
            const double stored = double(x86::Float(loaded));
            std::memcpy(unit, &stored, 8);
        }
        // 32 at a time: sub edx, 0x20 until it goes negative; then 8 at a time
        left -= 0x20;
        if (left >= 0)
        {
            // one fill for the whole run of 32-byte blocks
            const x86::reg32 blocks = x86::reg32(left) / 0x20 + 1;
            x86::reg8* to = memory + at;
            for (x86::reg32 i = 0; write && i < blocks * 4; ++i)
                std::memcpy(to + i * 8, unit, 8);
            at += blocks * 0x20;
            left -= x86::sreg32(blocks * 0x20);
        }
        left += 0x18;
        while (left >= 0)
        {
            if (write)
                std::memcpy(memory + at, unit, 8);
            at += 8;
            left -= 8;
        }
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
            {
                if (write)
                    memory[at] = x86::reg8(pattern);  // the last byte: eax not moved past it
            }
            flags.logic(x86::reg32(left) & 1);  // test edx, 1
        }
    }
    return {at, x86::reg32(left), flags};
}

x86::reg32 fillCore(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const FillResult r = fillRun(&app->getMemory<x86::reg8>(0), args.eax, x86::sreg32(args.edx), args.ebx, true);
    args.eax = r.at;
    args.edx = r.left;
    return args.eax;
}

}  // namespace

/* 0x4e0721's core: `count` bytes at `at` of the pattern; the eax it leaves (where the fill stopped). */
x86::reg32 ea::fillPattern(win32::WinApplication* app, x86::reg32 at, x86::reg32 count, x86::reg32 pattern)
{
    return fillRun(&app->getMemory<x86::reg8>(0), at, x86::sreg32(count), pattern, true).at;
}

/* 0x4e0721: edx bytes at eax of the pattern in ebx. */
x86::reg32 fillWord(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    fillCore(app, cpu, args);
    cpu.esp += 4;
    return args.eax;
}

/* 0x4e0716: edx bytes at eax of bl. */
x86::reg32 fillByte(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    x86::reg32 ebx = (args.ebx & 0xffff0000) | (args.ebx & 0xff) << 8 | (args.ebx & 0xff);
    ebx |= ebx << 16;
    args.ebx = ebx;
    fillCore(app, cpu, args);
    cpu.esp += 4;
    return args.eax;
}

/* 0x4e070c: edx bytes at eax cleared. */
x86::reg32 fillZero(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    const x86::reg32 ebx = args.ebx;
    args.ebx = 0;
    fillCore(app, cpu, args);
    args.ebx = ebx;
    cpu.esp += 4;
    return args.eax;
}

/* fillCore's bytes when all four of the pattern are one value: each step, the
 * alignment's, the 32 and 8 at a time and the tail's, writes that value, and
 * the steps together cover the count exactly -- the x87's load and store change
 * no such double (a NaN would need 0xff, which takes the dwords' path).  A count
 * with its top bit set fills nothing, as fillCore's signed tests have it. */
x86::reg32 ea::clearBytes(win32::WinApplication* app, x86::reg32 at, x86::reg32 count)
{
    if (x86::sreg32(count) > 0)
        std::memset(&app->getMemory<x86::reg8>(at), 0, count);
    return fillRun(nullptr, at, x86::sreg32(count), 0, false).at;  // the eax the fill leaves
}

void ea::fillBytes(win32::WinApplication* app, x86::reg32 at, x86::reg32 count, x86::reg32 value)
{
    if (x86::sreg32(count) > 0)
        std::memset(&app->getMemory<x86::reg8>(at), int(value & 0xff), count);
}

std::optional<x86::reg32> ea::messageOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 format)
{
    // As ea::error: the format pushed where the game's call pushes it, over the return address's slot.
    cpu.esp += 4;
    x86::Args registers;
    const std::optional<x86::reg32> eax = thunkCall(app, cpu, &Message, registers, {}, guest::Words({format}));
    if (!eax)
        return std::nullopt;
    cpu.esp -= 4;
    return eax;
}

std::optional<x86::reg32> ea::errorLeaves(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 eax, x86::reg32 format,
                                          guest::Words words)
{
    // As ea::error, with the eax the caller has at the call and what Error leaves there when it returns.
    x86::Args registers;
    registers.eax = eax;
    cpu.esp += 4;
    const std::optional<x86::reg32> left = thunkCall(app, cpu, &Error, registers, {format}, words);
    if (!left)
        return std::nullopt;
    cpu.esp -= 4;
    return left;
}

void ea::error(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 format, guest::Words words)
{
    // Called as C++ (guest::direct), the return address's slot already under esp: the words pushed
    // where the game's call pushes them, over that slot, so Error's frame lies where its code has it.
    cpu.esp += 4;
    x86::Args registers;
    const bool returned = thunkCall(app, cpu, &Error, registers, {format}, words).has_value();
    if (returned)
        cpu.esp -= 4;
}


/* The game asked the processor what it could do and timed it, to choose what to
 * run on it.  No phone needs that asked: what is left of the answers is what the
 * sound's set-up and the memmove read, MMX's routines (native_mix.cpp). */
namespace removed
{

void checkProcessor(win32::WinApplication* app)
{
    app->getMemory<x86::reg32>(0x5643b4) = 1;  // MMX's set of the mixing's routines
    app->getMemory<x86::reg32>(0x5643ac) = 1;
    app->getMemory<x86::reg32>(0x5643bc) = 0;  // no timestamp counter to read
}

void timeProcessor(win32::WinApplication* app)
{
    NFS2_USE(app);
}

x86::reg32 processorChecked(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    checkProcessor(app);
    cpu.esp += 4;
    return args.eax;
}

x86::reg32 processorSpeed(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    timeProcessor(app);
    cpu.esp += 4;
    return args.eax;
}

}  // namespace removed

}  // namespace nfs3hp
