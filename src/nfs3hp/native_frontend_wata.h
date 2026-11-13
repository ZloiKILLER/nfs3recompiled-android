#ifndef NFS3HP_NATIVE_FRONTEND_WATA_H
#define NFS3HP_NATIVE_FRONTEND_WATA_H

#include "native_frontend.h"
#include "native_guest.h"
#include <cstdint>

/* native_frontend_wata.cpp's typed functions (carp.txt's reader, NPS's DLLs, the install file and
 * the disc, the asynchronous file reader, the timer, the mixers' and blitter's front doors, the
 * window's and text's small functions, the HUD editor's drag and drop), which other files call as
 * C++ (guest::direct), and how the game's code calls each by address (ByAddress, for the checks'
 * stand-ins).  Each calls the game's other functions by address on `cpu` and throws fe::Terminated
 * when the game stopped in one, as Guest::call does; its result is what the game's function leaves
 * in eax.  The registers a function reads as it comes in are its parameters (Watcom's order: eax,
 * edx, ebx, ecx, then the stack's words); what it hands on in them to a call or a jump, it puts back
 * there itself. */
namespace nfs3hp
{
namespace frontend
{
namespace wata
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;
using sreg32 = x86::sreg32;

// The game's 0x4b0700: the wanted integer of a line of values.
reg32 carDataReadInteger(App* app, CPU& cpu, reg32 line, reg32 wanted);
// The game's 0x4b07b0: the wanted integer of a line of values, for a field the caller keeps a byte of.
reg32 carDataReadByte(App* app, CPU& cpu, reg32 line, reg32 wanted);
// The game's 0x4b07d0: the wanted float of a line of values on the x87, its bits in eax.
reg32 carDataReadFloat(App* app, CPU& cpu, reg32 line, reg32 wanted);
// The game's 0x4b08b0: a float of the file as 16.16 fixed point.
reg32 carDataReadFixed(App* app, CPU& cpu, reg32 line, reg32 wanted);
// The game's 0x4b0900: whether a line starts with a digit or a minus sign: values, not a key.
reg32 carDataLineHasValues(App* app, CPU& cpu, reg32 line);
// The game's 0x4b0940: which field a line of the file is the key of.
reg32 carDataFindKey(App* app, CPU& cpu, reg32 line, reg32 indexOut);
// The game's 0x4b09d0: the next line of the file, lowered and without its spaces, into `out`.
reg32 carDataNextLine(App* app, CPU& cpu, reg32 text, reg32 out, reg32 length);
// The game's 0x4b0bb0: the fields the file did not give, from the default record at 0x55f874; or, with edx not 0.
reg32 carDataFillDefaults(App* app, CPU& cpu, reg32 record, reg32 whole);
// The game's 0x4b1380: carp.txt into a car's performance record: lines of keys, each followed by a line of values.
reg32 carDataRead(App* app, CPU& cpu, reg32 record, reg32 text, reg32 length, reg32 firstOut);
// The game's 0x4a3070: an exception's name for the crash report.
reg32 exceptionName(App* app, CPU& cpu, reg32 code);
// The game's 0x4a54f0: an NPS DLL loaded, and with edx 0 every function of it looked up into its slot.
reg32 npsLoadDll(App* app, CPU& cpu, reg32 path, reg32 type);
// The game's 0x4a5780: whether the registry's NPS key says this is a developer's install.
reg32 npsDevelopMode(App* app, CPU& cpu);
// The game's 0x4a5880: the path of an NPS DLL from the registry, at 0x7a2020, with backslashes.
reg32 npsDllPath(App* app, CPU& cpu, reg32 type);
// The game's 0x4a59c0: an NPS DLL of a type loaded once; a message box if it cannot be.
reg32 npsEnsureDll(App* app, CPU& cpu, reg32 type);
// The game's 0x4a6e10: install.win read: its lines into the table at 0x7a2634.
reg32 installFileRead(App* app, CPU& cpu);
// The game's 0x4f0330: findfirst: the first file, not a folder, a pattern finds, its path at 0x9f0ac9.
reg32 findFirstFile(App* app, CPU& cpu, reg32 pattern);
// The game's 0x4f0280: findnext: the next file, not a folder, or 0 and the search closed.
reg32 findNextFile(App* app, CPU& cpu);
// The game's 0x4a46f0: the colour of a car's badge in the showcase, by the words in its name.
reg32 carBadgeColour(App* app, CPU& cpu);
// The game's 0x4f4920: a field of a window's description within its limits; an error if not and the check is on.
reg32 windowCheckField(App* app, CPU& cpu,
                       reg32 window, reg32 value, reg32 name, reg32 least, reg32 most, reg32 wanted);
// The game's 0x4f4a80: a window's description checked, field by field: 1 when all are in their limits.
reg32 windowCheck(App* app, CPU& cpu, reg32 window);
// The game's 0x4f9390: a directory joined to a file's name in a buffer of the size given: 0 if it will not fit.
reg32 pathInDirectory(App* app, CPU& cpu, reg32 buffer, reg32 fileName, reg32 size);
// The game's 0x4f0b60: a string run through the text's own measure (sub_4efb50) with its output turned aside.
reg32 textMeasureBox(App* app, CPU& cpu,
                     reg32 text, sreg32 given, reg32 leftOut, reg32 topOut, reg32 widthOut, reg32 heightOut);
// The game's 0x4fb710: a shift key's lock light set to what is asked, by pressing it: 0 caps, 1 num, 2 scroll.
reg32 lockKeySet(App* app, CPU& cpu, reg32 lock, reg32 wanted);
// The game's 0x4a5080: the game's command line, last word first.
reg32 commandLineRead(App* app, CPU& cpu, reg32 count, reg32 words);
// The game's 0x4b6320: the game's disc looked for, a box and the end if it is run from one or there is none.
reg32 discCheck(App* app, CPU& cpu);
// The game's 0x4f9410: whether a drive, 0 for A:, is a CD.
reg32 driveIsCd(App* app, CPU& cpu, reg32 drive);
// The game's 0x4f93f0: a path's drive, 0 for A:, or -1.
reg32 driveNumberOfPath(App* app, CPU& cpu, reg32 path);
// The game's 0x4f0cb0: a queue's search: whether a request has this id.
reg32 asyncRequestHasId(App* app, CPU& cpu, reg32 request, reg32 id);
// The game's 0x4f0cc0: nothing found.
reg32 asyncNoRequest(App* app, CPU& cpu);
// The game's 0x4f0cd0: what the thread is at told to stop, and waited for.
reg32 asyncCancelCurrent(App* app, CPU& cpu);
// The game's 0x4f0d20: the system task: finished requests' callbacks, in the game's own thread.
reg32 asyncRunCallbacks(App* app, CPU& cpu);
// The game's 0x4f0da0: the reader started, with room for twice this many requests: its record, its queues.
reg32 asyncInit(App* app, CPU& cpu, reg32 requests);
// The game's 0x4f0f40: the reader stopped and everything it holds freed.
reg32 asyncShutdown(App* app, CPU& cpu);
// The game's 0x4f1370: the file the reader reads from next, by name; 0 for none.
reg32 asyncSetFile(App* app, CPU& cpu, reg32 name);
// The game's 0x4f1500: a read of the open file asked for, with a callback or none: its id, 0 with no free request.
reg32 asyncReadAt(App* app, CPU& cpu, reg32 offset, reg32 buffer, reg32 size, reg32 callback);
// The game's 0x4f15d0: a read with no callback.
reg32 asyncRead(App* app, CPU& cpu, reg32 offset, reg32 buffer, reg32 size);
// The game's 0x4f1790: a read's end: 1 done, -1 failed, 0 not yet.
reg32 asyncReadStatus(App* app, CPU& cpu, reg32 id);
// The game's 0x4f1820: the reader's thread: a request at a time from the waiting queue, until it is told to close.
reg32 asyncThreadMain(App* app, CPU& cpu);
// The game's 0x4f0630: the timer's ticks.
reg32 timerNow(App* app, CPU& cpu);
// The game's 0x4f0650: the ticks since the last time it was asked.
reg32 timerSinceLast(App* app, CPU& cpu);
// The game's 0x4f06e0: ticks waited for, the system's tasks run meanwhile.
reg32 timerWait(App* app, CPU& cpu, reg32 ticks);
// The game's 0x4f0840: the screen's colour depth, as the text keeps it.
reg32 textDepth(App* app, CPU& cpu);
// The game's 0x4f0850: the text's pen.
reg32 textSetPen(App* app, CPU& cpu, reg32 x, reg32 y);
// The game's 0x4f0860: the text's attribute set, and on to what follows it.
reg32 textSetAttribute(App* app, CPU& cpu, reg32 value);
// The game's 0x4f0880: mixer A's front door for 4-bit samples: the count (ebx) made bytes.
reg32 mixA4Bit(App* app, CPU& cpu, reg32 count);
// The game's 0x4f0890: mixer A's front door for 16-bit samples: the count (ebx) made bytes.
reg32 mixA16Bit(App* app, CPU& cpu, reg32 count);
// The game's 0x4f08c0: mixer A's front door for 32-bit samples: the count (ebx) made bytes.
reg32 mixA32Bit(App* app, CPU& cpu, reg32 count);
// The game's 0x4f08d0: mixer B's front door for 4-bit samples: the count (ebx) made bytes.
reg32 mixB4Bit(App* app, CPU& cpu, reg32 count);
// The game's 0x4f08e0: mixer B's front door for 16-bit samples: the count (ebx) made bytes.
reg32 mixB16Bit(App* app, CPU& cpu, reg32 count);
// The game's 0x4f0910: mixer B's front door for 32-bit samples: the count (ebx) made bytes.
reg32 mixB32Bit(App* app, CPU& cpu, reg32 count);
// The game's 0x4f0920: mixer C's front door for 4-bit samples: the count (ebx) made bytes.
reg32 mixC4Bit(App* app, CPU& cpu, reg32 count);
// The game's 0x4f0930: mixer C's front door for 16-bit samples: the count (ebx) made bytes.
reg32 mixC16Bit(App* app, CPU& cpu, reg32 count);
// The game's 0x4f0960: mixer C's front door for 32-bit samples: the count (ebx) made bytes.
reg32 mixC32Bit(App* app, CPU& cpu, reg32 count);
// The game's 0x4f0970: mixer D's front door for 4-bit samples: the count (ebx) made bytes.
reg32 mixD4Bit(App* app, CPU& cpu, reg32 count);
// The game's 0x4f0980: mixer D's front door for 16-bit samples: the count (ebx) made bytes.
reg32 mixD16Bit(App* app, CPU& cpu, reg32 count);
// The game's 0x4f09b0: mixer D's front door for 32-bit samples: the count (ebx) made bytes.
reg32 mixD32Bit(App* app, CPU& cpu, reg32 count);
// The game's 0x4f08a0: mixer A's front door for 24-bit samples: the count (ebx) made bytes.
reg32 mixA24Bit(App* app, CPU& cpu, reg32 samples, reg32 count);
// The game's 0x4f08f0: mixer B's front door for 24-bit samples: the count (ebx) made bytes.
reg32 mixB24Bit(App* app, CPU& cpu, reg32 samples, reg32 count);
// The game's 0x4f0940: mixer C's front door for 24-bit samples: the count (ebx) made bytes.
reg32 mixC24Bit(App* app, CPU& cpu, reg32 samples, reg32 count);
// The game's 0x4f0990: mixer D's front door for 24-bit samples: the count (ebx) made bytes.
reg32 mixD24Bit(App* app, CPU& cpu, reg32 samples, reg32 count);
// The game's 0x4f09c0: the blitter that suits the processor, chosen the first time, called.
reg32 blitDispatch(App* app, CPU& cpu);
// The game's 0x4f0a10: the blitter, the two 12-bit fields of the shape's position word in ebx and edx.
reg32 blitShapeAtPosition(App* app, CPU& cpu, reg32 shape);
// The game's 0x4f0a90: a surface filled with a colour and drawn.
reg32 surfaceFillAndDraw(App* app, CPU& cpu, reg32 x, reg32 y, reg32 width, reg32 height, reg32 colour);
// The game's 0x4f0af0: the text's measure, in place of its drawing: the box grown by a glyph.
reg32 textMeasureGlyph(App* app, CPU& cpu, sreg32 x, sreg32 y, sreg32 width, sreg32 height);
// The game's 0x4f0c90: sub_4f0b60 with room for the whole string.
reg32 textMeasure(App* app, CPU& cpu, reg32 text, reg32 leftOut, reg32 topOut, reg32 widthOut, reg32 heightOut);
// The game's 0x4f2170: how many times round a loop in so many ticks: the processor's speed.
reg32 processorCountLoops(App* app, CPU& cpu, sreg32 ticks);
// The game's 0x4f4650: a string and a value into the next row of the table at 0x79f524.
reg32 registryAddEntry(App* app, CPU& cpu, reg32 name, reg32 value);
// The game's 0x4f46b0: the current screen's surface's pixels.
reg32 screenPixels(App* app, CPU& cpu);
// The game's 0x4f46d0: the window's two descriptions, kept.
reg32 windowSave(App* app, CPU& cpu, reg32 to);
// The game's 0x4f4700: the window's descriptions put back as sub_4f46d0 kept them: the check off, for it.
reg32 windowRestore(App* app, CPU& cpu, reg32 from);
// The game's 0x4f4740: the text's whole state, kept.
reg32 textStateSave(App* app, CPU& cpu, reg32 to);
// The game's 0x4f4760: and put back.
reg32 textStateRestore(App* app, CPU& cpu, reg32 from);
// The game's 0x4f4780: the checksum of a block, by the table at 0x566b54.
reg32 blockChecksum(App* app, CPU& cpu, reg32 start, reg32 size);
// The game's 0x4f47c0: how a shape's file ends: 1 at a BEND, 0 not, 2 of a kind that has no such end.
reg32 shapeEndsAtBend(App* app, CPU& cpu, reg32 shape);
// The game's 0x4f87e0: a graphics driver's DLL opened: the name given, after a '!', or the environment's.
reg32 thrashOpenByName(App* app, CPU& cpu, reg32 given);
// The game's 0x4f90e0: the window shown again, its descriptions kept through it.
reg32 windowReshow(App* app, CPU& cpu);
// The game's 0x4f92b0: the heap's block allocated and handed over.
reg32 heapStart(App* app, CPU& cpu, reg32 size);
// The game's 0x4f92d0: 64 megabytes of it: tools/apply_heap_budget.py's, 16 in the game.
reg32 heapStartDefault(App* app, CPU& cpu);
// The game's 0x4f9320: the fonts' system started, once.
reg32 fontsStart(App* app, CPU& cpu);
// The game's 0x4f99f0: TAPI's line opened, the callback at 0x4f9500.
reg32 tapiInitialize(App* app, CPU& cpu, reg32 instance, reg32 wanted);
// The game's 0x4f9920: TAPI set up for the modem games: 0, or what went wrong.
reg32 tapiSetup(App* app, CPU& cpu, reg32 wanted);
// The game's 0x4fb380: the screen's size and depth from the driver's record into the global ones.
reg32 screenTakeSize(App* app, CPU& cpu);
// The game's 0x4fb3b0: the screen's lock count, one up.
reg32 screenLockCountUp(App* app, CPU& cpu);
// The game's 0x4fb3c0: the screen's lock count, one down.
reg32 screenLockCountDown(App* app, CPU& cpu);
// The game's 0x4fb3d0: the driver's record set up for the screen, and these functions hooked in.
reg32 screenInstallDriver(App* app, CPU& cpu);
// The game's 0x4fb700: the num lock off.
reg32 numLockOff(App* app, CPU& cpu);
// The game's 0x4fb790: a directory's search: a big file's entry opened by its name, 0 when found.
reg32 bigFileOpenEntry(App* app, CPU& cpu, reg32 big, reg32 search);
// The game's 0x4ead20: the window as it stands set: an error if it is locked.
reg32 windowApply(App* app, CPU& cpu);
// The game's 0x4f7c48: the exit functions registered at 0x9f380c run, last first, once.
reg32 exitFunctionsRun(App* app, CPU& cpu);
// The game's 0x4e0658: nothing, in a table of what can be hooked.
reg32 nothingA(App* app, CPU& cpu);
// The game's 0x4e0659: nothing, in a table of what can be hooked.
reg32 nothingB(App* app, CPU& cpu);
// The game's 0x4f45d0: nothing, in a table of what can be hooked.
reg32 nothingC(App* app, CPU& cpu);
// The game's 0x4f8930: the graphics started: the driver's settings in the environment the first time, the display mode.
reg32 graphicsStart(App* app, CPU& cpu);
// The game's 0x47a970: the sky's shape for a track: its description's words in the shape's name give the sun's direction.
reg32 skyLoad(App* app, CPU& cpu, reg32 shapes, reg32 shapeName);
// The game's 0x459fe0: an element dropped: where it now is into the layout's table, the HUD rebuilt.
reg32 hudEditorDrop(App* app, CPU& cpu, reg32 element, reg32 start);
// The game's 0x45a120: an element under the mouse, each frame.
reg32 hudEditorDrag(App* app, CPU& cpu, reg32 element);

}  // namespace wata
}  // namespace frontend

namespace guest
{

template <>
struct ByAddress<frontend::wata::carDataReadInteger>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 line, x86::reg32 wanted)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ line, wanted }));
    }
};

template <>
struct ByAddress<frontend::wata::carDataReadByte>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 line, x86::reg32 wanted)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ line, wanted }));
    }
};

template <>
struct ByAddress<frontend::wata::carDataReadFloat>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 line, x86::reg32 wanted)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ line, wanted }));
    }
};

template <>
struct ByAddress<frontend::wata::carDataReadFixed>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 line, x86::reg32 wanted)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ line, wanted }));
    }
};

template <>
struct ByAddress<frontend::wata::carDataLineHasValues>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 line)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ line }));
    }
};

template <>
struct ByAddress<frontend::wata::carDataFindKey>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 line, x86::reg32 indexOut)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ line, indexOut }));
    }
};

template <>
struct ByAddress<frontend::wata::carDataNextLine>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu,
                           x86::reg32 text, x86::reg32 out, x86::reg32 length)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ text, out, length }));
    }
};

template <>
struct ByAddress<frontend::wata::carDataFillDefaults>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 record, x86::reg32 whole)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ record, whole }));
    }
};

template <>
struct ByAddress<frontend::wata::carDataRead>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu,
                           x86::reg32 record, x86::reg32 text, x86::reg32 length, x86::reg32 firstOut)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ record, text, length, firstOut }));
    }
};

template <>
struct ByAddress<frontend::wata::exceptionName>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 code)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ code }));
    }
};

template <>
struct ByAddress<frontend::wata::npsLoadDll>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 path, x86::reg32 type)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ path, type }));
    }
};

template <>
struct ByAddress<frontend::wata::npsDevelopMode>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::npsDllPath>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 type)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ type }));
    }
};

template <>
struct ByAddress<frontend::wata::npsEnsureDll>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 type)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ type }));
    }
};

template <>
struct ByAddress<frontend::wata::installFileRead>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::findFirstFile>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 pattern)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ pattern }));
    }
};

template <>
struct ByAddress<frontend::wata::findNextFile>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::carBadgeColour>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::windowCheckField>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu,
                           x86::reg32 window, x86::reg32 value, x86::reg32 name, x86::reg32 least, x86::reg32 most, x86::reg32 wanted)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ window, value, name, least }, { most, wanted }));
    }
};

template <>
struct ByAddress<frontend::wata::windowCheck>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 window)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ window }));
    }
};

template <>
struct ByAddress<frontend::wata::pathInDirectory>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu,
                           x86::reg32 buffer, x86::reg32 fileName, x86::reg32 size)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ buffer, fileName, size }));
    }
};

template <>
struct ByAddress<frontend::wata::textMeasureBox>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu,
                           x86::reg32 text, x86::sreg32 given, x86::reg32 leftOut, x86::reg32 topOut, x86::reg32 widthOut, x86::reg32 heightOut)
    {
        return fe::callByAddress(app, cpu, Address,
                                 fe::args({ text, x86::reg32(given), leftOut, topOut }, { widthOut, heightOut }));
    }
};

template <>
struct ByAddress<frontend::wata::lockKeySet>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 lock, x86::reg32 wanted)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ lock, wanted }));
    }
};

template <>
struct ByAddress<frontend::wata::commandLineRead>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 count, x86::reg32 words)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ count, words }));
    }
};

template <>
struct ByAddress<frontend::wata::discCheck>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::driveIsCd>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 drive)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ drive }));
    }
};

template <>
struct ByAddress<frontend::wata::driveNumberOfPath>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 path)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ path }));
    }
};

template <>
struct ByAddress<frontend::wata::asyncRequestHasId>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 request, x86::reg32 id)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ request, id }));
    }
};

template <>
struct ByAddress<frontend::wata::asyncNoRequest>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::asyncCancelCurrent>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::asyncRunCallbacks>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::asyncInit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 requests)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ requests }));
    }
};

template <>
struct ByAddress<frontend::wata::asyncShutdown>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::asyncSetFile>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ name }));
    }
};

template <>
struct ByAddress<frontend::wata::asyncReadAt>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu,
                           x86::reg32 offset, x86::reg32 buffer, x86::reg32 size, x86::reg32 callback)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ offset, buffer, size, callback }));
    }
};

template <>
struct ByAddress<frontend::wata::asyncRead>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu,
                           x86::reg32 offset, x86::reg32 buffer, x86::reg32 size)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ offset, buffer, size }));
    }
};

template <>
struct ByAddress<frontend::wata::asyncReadStatus>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 id)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ id }));
    }
};

template <>
struct ByAddress<frontend::wata::asyncThreadMain>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::timerNow>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::timerSinceLast>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::timerWait>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 ticks)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ ticks }));
    }
};

template <>
struct ByAddress<frontend::wata::textDepth>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::textSetPen>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 x, x86::reg32 y)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ x, y }));
    }
};

template <>
struct ByAddress<frontend::wata::textSetAttribute>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 value)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ value }));
    }
};

template <>
struct ByAddress<frontend::wata::mixA4Bit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, cpu.edx, count }));
    }
};

template <>
struct ByAddress<frontend::wata::mixA16Bit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, cpu.edx, count }));
    }
};

template <>
struct ByAddress<frontend::wata::mixA32Bit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, cpu.edx, count }));
    }
};

template <>
struct ByAddress<frontend::wata::mixB4Bit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, cpu.edx, count }));
    }
};

template <>
struct ByAddress<frontend::wata::mixB16Bit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, cpu.edx, count }));
    }
};

template <>
struct ByAddress<frontend::wata::mixB32Bit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, cpu.edx, count }));
    }
};

template <>
struct ByAddress<frontend::wata::mixC4Bit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, cpu.edx, count }));
    }
};

template <>
struct ByAddress<frontend::wata::mixC16Bit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, cpu.edx, count }));
    }
};

template <>
struct ByAddress<frontend::wata::mixC32Bit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, cpu.edx, count }));
    }
};

template <>
struct ByAddress<frontend::wata::mixD4Bit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, cpu.edx, count }));
    }
};

template <>
struct ByAddress<frontend::wata::mixD16Bit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, cpu.edx, count }));
    }
};

template <>
struct ByAddress<frontend::wata::mixD32Bit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, cpu.edx, count }));
    }
};

template <>
struct ByAddress<frontend::wata::mixA24Bit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 samples, x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ samples, cpu.edx, count }));
    }
};

template <>
struct ByAddress<frontend::wata::mixB24Bit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 samples, x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ samples, cpu.edx, count }));
    }
};

template <>
struct ByAddress<frontend::wata::mixC24Bit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 samples, x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ samples, cpu.edx, count }));
    }
};

template <>
struct ByAddress<frontend::wata::mixD24Bit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 samples, x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ samples, cpu.edx, count }));
    }
};

template <>
struct ByAddress<frontend::wata::blitDispatch>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::blitShapeAtPosition>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 shape)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ shape }));
    }
};

template <>
struct ByAddress<frontend::wata::surfaceFillAndDraw>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu,
                           x86::reg32 x, x86::reg32 y, x86::reg32 width, x86::reg32 height, x86::reg32 colour)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ x, y, width, height }, { colour }));
    }
};

template <>
struct ByAddress<frontend::wata::textMeasureGlyph>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu,
                           x86::sreg32 x, x86::sreg32 y, x86::sreg32 width, x86::sreg32 height)
    {
        return fe::callByAddress(app, cpu, Address,
                                 fe::args({ x86::reg32(x), x86::reg32(y) }, { 0, x86::reg32(width), x86::reg32(height) }));
    }
};

template <>
struct ByAddress<frontend::wata::textMeasure>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu,
                           x86::reg32 text, x86::reg32 leftOut, x86::reg32 topOut, x86::reg32 widthOut, x86::reg32 heightOut)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ text, leftOut, topOut, widthOut }, { heightOut }));
    }
};

template <>
struct ByAddress<frontend::wata::processorCountLoops>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::sreg32 ticks)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ x86::reg32(ticks) }));
    }
};

template <>
struct ByAddress<frontend::wata::registryAddEntry>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name, x86::reg32 value)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ name, value }));
    }
};

template <>
struct ByAddress<frontend::wata::screenPixels>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::windowSave>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 to)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ to }));
    }
};

template <>
struct ByAddress<frontend::wata::windowRestore>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 from)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ from }));
    }
};

template <>
struct ByAddress<frontend::wata::textStateSave>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 to)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ to }));
    }
};

template <>
struct ByAddress<frontend::wata::textStateRestore>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 from)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ from }));
    }
};

template <>
struct ByAddress<frontend::wata::blockChecksum>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 start, x86::reg32 size)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ start, size }));
    }
};

template <>
struct ByAddress<frontend::wata::shapeEndsAtBend>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 shape)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ shape }));
    }
};

template <>
struct ByAddress<frontend::wata::thrashOpenByName>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 given)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ given }));
    }
};

template <>
struct ByAddress<frontend::wata::windowReshow>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::heapStart>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 size)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ size }));
    }
};

template <>
struct ByAddress<frontend::wata::heapStartDefault>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::fontsStart>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::tapiInitialize>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 instance, x86::reg32 wanted)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ instance, wanted }));
    }
};

template <>
struct ByAddress<frontend::wata::tapiSetup>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 wanted)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ wanted }));
    }
};

template <>
struct ByAddress<frontend::wata::screenTakeSize>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::screenLockCountUp>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::screenLockCountDown>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::screenInstallDriver>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::numLockOff>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::bigFileOpenEntry>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 big, x86::reg32 search)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ big, search }));
    }
};

template <>
struct ByAddress<frontend::wata::windowApply>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::exitFunctionsRun>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::nothingA>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::nothingB>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::nothingC>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::graphicsStart>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wata::skyLoad>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 shapes, x86::reg32 shapeName)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ shapes, shapeName }));
    }
};

template <>
struct ByAddress<frontend::wata::hudEditorDrop>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 element, x86::reg32 start)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ element, start }));
    }
};

template <>
struct ByAddress<frontend::wata::hudEditorDrag>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 element)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ element }));
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
