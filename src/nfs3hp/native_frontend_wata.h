#ifndef NFS3HP_NATIVE_FRONTEND_WATA_H
#define NFS3HP_NATIVE_FRONTEND_WATA_H

#include "native_frontend.h"
#include "native_guest.h"
#include <cstdint>
#include <cstring>
#include <optional>

/* native_frontend_wata.cpp's typed functions (carp.txt's reader, NPS's DLLs, the install file and
 * the disc, the asynchronous file reader, the timer, the mixers' and blitter's front doors, the
 * window's and text's small functions, the HUD editor's drag and drop), which other files call as
 * C++ (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions as C++ where they have typed cores, by address
 * otherwise, and throws fe::Terminated when the game stopped in one, as a call by address does.  The
 * registers a function reads as it comes in, and those it hands on as they came to a call or a
 * jump, are its parameters (Watcom's order: eax, edx, ebx, ecx, then the stack's words); its result
 * is what the game's function hands back in eax.  What the game's code leaves in the registers it
 * does not save is not kept. */
namespace nfs3hp
{
namespace frontend
{
namespace wata
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;
using s32 = nfs::s32;

// The game's 0x4b0700: the wanted integer of a line of values.
u32 carDataReadInteger(App* app, ThreadState& ts, u32 line, u32 wanted);
// The game's 0x4b07b0: the wanted integer of a line of values, for a field the caller keeps a byte of.
u32 carDataReadByte(App* app, ThreadState& ts, u32 line, u32 wanted);
// The game's 0x4b07d0: the wanted float of a line of values: its bits (eax); the float itself its code leaves on
// the x87's stack.
u32 carDataReadFloat(App* app, ThreadState& ts, u32 line, u32 wanted);
// The game's 0x4b08b0: a float of the file as 16.16 fixed point.
u32 carDataReadFixed(App* app, ThreadState& ts, u32 line, u32 wanted);
// The game's 0x4b0900: whether a line starts with a digit or a minus sign: values, not a key.
u32 carDataLineHasValues(App* app, ThreadState& ts, u32 line);
// The game's 0x4b0940: which field a line of the file is the key of.
u32 carDataFindKey(App* app, ThreadState& ts, u32 line, u32 indexOut);
// The game's 0x4b09d0: the next line of the file, lowered and without its spaces, into `out`.
u32 carDataNextLine(App* app, ThreadState& ts, u32 text, u32 out, u32 length);
// The game's 0x4b0bb0: the fields the file did not give, from the default record at 0x55f874; or, with edx not 0.
u32 carDataFillDefaults(App* app, ThreadState& ts, u32 record, u32 whole);
// The game's 0x4b1380: carp.txt into a car's performance record: lines of keys, each followed by a line of values.
u32 carDataRead(App* app, ThreadState& ts, u32 record, u32 text, u32 length, u32 firstOut);
// The game's 0x4a3070: an exception's name for the crash report.
u32 exceptionName(App* app, ThreadState& ts, u32 code);
// The game's 0x4a54f0: an NPS DLL loaded, and with edx 0 every function of it looked up into its slot.
u32 npsLoadDll(App* app, ThreadState& ts, u32 path, u32 type);
// The game's 0x4a5780: whether the registry's NPS key says this is a developer's install.
u32 npsDevelopMode(App* app, ThreadState& ts);
// The game's 0x4a5880: the path of an NPS DLL from the registry, at 0x7a2020, with backslashes.
u32 npsDllPath(App* app, ThreadState& ts, u32 type);
// The game's 0x4a59c0: an NPS DLL of a type loaded once; a message box if it cannot be.
u32 npsEnsureDll(App* app, ThreadState& ts, u32 type);
// The game's 0x4a6e10: install.win read: its lines into the table at 0x7a2634.
u32 installFileRead(App* app, ThreadState& ts);
// The game's 0x4f0330: findfirst: the first file, not a folder, a pattern finds, its path at 0x9f0ac9.
u32 findFirstFile(App* app, ThreadState& ts, u32 pattern);
// The game's 0x4f0280: findnext: the next file, not a folder, or 0 and the search closed.
u32 findNextFile(App* app, ThreadState& ts);
// The game's 0x4a46f0: the colour of a car's badge in the showcase, by the words in its name.
u32 carBadgeColour(App* app, ThreadState& ts);
// The game's 0x4f4920: a field of a window's description within its limits; an error if not and the check is on.
u32 windowCheckField(App* app, ThreadState& ts,
                       u32 window, u32 value, u32 name, u32 least, u32 most, u32 wanted);
// The game's 0x4f4a80: a window's description checked, field by field: 1 when all are in their limits.
u32 windowCheck(App* app, ThreadState& ts, u32 window);
// The game's 0x4f9390: a directory joined to a file's name in a buffer of the size given: 0 if it will not fit.
u32 pathInDirectory(App* app, ThreadState& ts, u32 buffer, u32 fileName, u32 size);
// The game's 0x4f0b60: a string run through the text's own measure (sub_4efb50) with its output turned aside.
u32 textMeasureBox(App* app, ThreadState& ts,
                     u32 text, s32 given, u32 leftOut, u32 topOut, u32 widthOut, u32 heightOut);
// The game's 0x4fb710: a shift key's lock light set to what is asked, by pressing it: 0 caps, 1 num, 2 scroll.
u32 lockKeySet(App* app, ThreadState& ts, u32 lock, u32 wanted);
// The game's 0x4a5080: the game's command line, last word first.
u32 commandLineRead(App* app, ThreadState& ts, u32 count, u32 words);
// The game's 0x4b6320: the game's disc looked for, a box and the end if it is run from one or there is none.
u32 discCheck(App* app, ThreadState& ts);
// The game's 0x4f9410: whether a drive, 0 for A:, is a CD.
u32 driveIsCd(App* app, ThreadState& ts, u32 drive);
// The game's 0x4f93f0: a path's drive, 0 for A:, or -1.
u32 driveNumberOfPath(App* app, ThreadState& ts, u32 path);
// The game's 0x4f0cb0: a queue's search: whether a request has this id.
u32 asyncRequestHasId(App* app, ThreadState& ts, u32 request, u32 id);
// The game's 0x4f0cc0: nothing found.
u32 asyncNoRequest(App* app, ThreadState& ts);
// The game's 0x4f0cd0: what the thread is at told to stop, and waited for.
u32 asyncCancelCurrent(App* app, ThreadState& ts);
// The game's 0x4f0d20: the system task: finished requests' callbacks, in the game's own thread.
u32 asyncRunCallbacks(App* app, ThreadState& ts);
// The game's 0x4f0da0: the reader started, with room for twice this many requests: its record, its queues; `memoryClass`, the caller\'s ebx, its record\'s class.
u32 asyncInit(App* app, ThreadState& ts, u32 requests, u32 memoryClass);
// The game's 0x4f0f40: the reader stopped and everything it holds freed.
u32 asyncShutdown(App* app, ThreadState& ts);
// The game's 0x4f1370: the file the reader reads from next, by name; 0 for none.
u32 asyncSetFile(App* app, ThreadState& ts, u32 name);
// The game's 0x4f1500: a read of the open file asked for, with a callback or none: its id, 0 with no free request.
u32 asyncReadAt(App* app, ThreadState& ts, u32 offset, u32 buffer, u32 size, u32 callback);
// The game's 0x4f15d0: a read with no callback.
u32 asyncRead(App* app, ThreadState& ts, u32 offset, u32 buffer, u32 size);
// The game's 0x4f1790: a read's end: 1 done, -1 failed, 0 not yet.
u32 asyncReadStatus(App* app, ThreadState& ts, u32 id);
// The game's 0x4f1820: the reader's thread: a request at a time from the waiting queue, until it is told to close.
u32 asyncThreadMain(App* app, ThreadState& ts);
// The game's 0x4f0630: the timer's ticks.
u32 timerNow(App* app, ThreadState& ts);
// The game's 0x4f0650: the ticks since the last time it was asked.
u32 timerSinceLast(App* app, ThreadState& ts);
// The game's 0x4f06e0: ticks waited for, the system's tasks run meanwhile.
u32 timerWait(App* app, ThreadState& ts, u32 ticks);
// The game's 0x4f0840: the screen's colour depth, as the text keeps it.
u32 textDepth(App* app, ThreadState& ts);
// The game's 0x4f0850: the text's pen.
u32 textSetPen(App* app, ThreadState& ts, u32 x, u32 y);
// The game's 0x4f0860: the text's attribute set, and on to what follows it.
u32 textSetAttribute(App* app, ThreadState& ts, u32 value);
// The game's 0x4f0880: mixer A's front door for 4-bit samples (eax) going to edx: the count (ebx) made bytes.
u32 mixA4Bit(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x4f0890: mixer A's front door for 16-bit samples (eax) going to edx: the count (ebx) made bytes.
u32 mixA16Bit(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x4f08c0: mixer A's front door for 32-bit samples (eax) going to edx: the count (ebx) made bytes.
u32 mixA32Bit(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x4f08d0: mixer B's front door for 4-bit samples (eax) going to edx: the count (ebx) made bytes.
u32 mixB4Bit(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x4f08e0: mixer B's front door for 16-bit samples (eax) going to edx: the count (ebx) made bytes.
u32 mixB16Bit(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x4f0910: mixer B's front door for 32-bit samples (eax) going to edx: the count (ebx) made bytes.
u32 mixB32Bit(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x4f0920: mixer C's front door for 4-bit samples (eax) going to edx: the count (ebx) made bytes.
u32 mixC4Bit(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x4f0930: mixer C's front door for 16-bit samples (eax) going to edx: the count (ebx) made bytes.
u32 mixC16Bit(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x4f0960: mixer C's front door for 32-bit samples (eax) going to edx: the count (ebx) made bytes.
u32 mixC32Bit(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x4f0970: mixer D's front door for 4-bit samples (eax) going to edx: the count (ebx) made bytes.
u32 mixD4Bit(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x4f0980: mixer D's front door for 16-bit samples (eax) going to edx: the count (ebx) made bytes.
u32 mixD16Bit(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x4f09b0: mixer D's front door for 32-bit samples (eax) going to edx: the count (ebx) made bytes.
u32 mixD32Bit(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x4f08a0: mixer A's front door for 24-bit samples (eax) going to edx: the count (ebx) made bytes.
u32 mixA24Bit(App* app, ThreadState& ts, u32 samples, u32 to, u32 count);
// The game's 0x4f08f0: mixer B's front door for 24-bit samples (eax) going to edx: the count (ebx) made bytes.
u32 mixB24Bit(App* app, ThreadState& ts, u32 samples, u32 to, u32 count);
// The game's 0x4f0940: mixer C's front door for 24-bit samples (eax) going to edx: the count (ebx) made bytes.
u32 mixC24Bit(App* app, ThreadState& ts, u32 samples, u32 to, u32 count);
// The game's 0x4f0990: mixer D's front door for 24-bit samples (eax) going to edx: the count (ebx) made bytes.
u32 mixD24Bit(App* app, ThreadState& ts, u32 samples, u32 to, u32 count);
// The game's 0x4f09c0: the blitter that suits the processor, chosen the first time, called: the shape at x, y.
u32 blitDispatchAt(App* app, ThreadState& ts, u32 shape, u32 x, u32 y);
// The game's 0x4f0a10: the blitter, the place the two 12-bit fields of the shape\'s position word.
u32 blitShapeAtPosition(App* app, ThreadState& ts, u32 shape);
// The game's 0x4f0a90: a surface filled with a colour and drawn.
u32 surfaceFillAndDraw(App* app, ThreadState& ts, u32 x, u32 y, u32 width, u32 height, u32 colour);
// The game's 0x4f0af0: the text's measure, in place of its drawing: the box grown by a glyph.
u32 textMeasureGlyph(App* app, ThreadState& ts, s32 x, s32 y, s32 width, s32 height);
// The game's 0x4f0c90: sub_4f0b60 with room for the whole string.
u32 textMeasure(App* app, ThreadState& ts, u32 text, u32 leftOut, u32 topOut, u32 widthOut, u32 heightOut);
// The game's 0x4f2170: how many times round a loop in so many ticks: the processor's speed.
u32 processorCountLoops(App* app, ThreadState& ts, s32 ticks);
// The game's 0x4f4650: a string and a value into the next row of the table at 0x79f524.
u32 registryAddEntry(App* app, ThreadState& ts, u32 name, u32 value);
// The game's 0x4f46b0: the current screen's surface's pixels.
u32 screenPixels(App* app, ThreadState& ts);
// The game's 0x4f46d0: the window's two descriptions, kept.
u32 windowSave(App* app, ThreadState& ts, u32 to);
// The game's 0x4f4700: the window's descriptions put back as sub_4f46d0 kept them: the check off, for it.
u32 windowRestore(App* app, ThreadState& ts, u32 from);
// The game's 0x4f4740: the text's whole state, kept.
u32 textStateSave(App* app, ThreadState& ts, u32 to);
// The game's 0x4f4760: and put back.
u32 textStateRestore(App* app, ThreadState& ts, u32 from);
// The game's 0x4f4780: the checksum of a block, by the table at 0x566b54.
u32 blockChecksum(App* app, ThreadState& ts, u32 start, u32 size);
// The game's 0x4f47c0: how a shape's file ends: 1 at a BEND, 0 not, 2 of a kind that has no such end.
u32 shapeEndsAtBend(App* app, ThreadState& ts, u32 shape);
// The game's 0x4f87e0: a graphics driver's DLL opened: the name given, after a '!', or the environment's.
u32 thrashOpenByName(App* app, ThreadState& ts, u32 given);
// The game's 0x4f90e0: the window shown again, its descriptions kept through it.
u32 windowReshow(App* app, ThreadState& ts);
// The game's 0x4f92b0: the heap's block allocated and handed over.
u32 heapStart(App* app, ThreadState& ts, u32 size);
// The game's 0x4f92d0: 64 megabytes of it: the port's heap budget change's, 16 in the game.
u32 heapStartDefault(App* app, ThreadState& ts);
// The game's 0x4f9320: the fonts' system started, once.
u32 fontsStart(App* app, ThreadState& ts);
// The game's 0x4f99f0: TAPI's line opened, the callback at 0x4f9500.
u32 tapiInitialize(App* app, ThreadState& ts, u32 instance, u32 wanted);
// The game's 0x4f9920: TAPI set up for the modem games: 0, or what went wrong.
u32 tapiSetup(App* app, ThreadState& ts, u32 wanted);
// The game's 0x4fb380: the screen's size and depth from the driver's record into the global ones.
u32 screenTakeSize(App* app, ThreadState& ts);
// The game's 0x4fb3b0: the screen's lock count, one up.
u32 screenLockCountUp(App* app, ThreadState& ts);
// The game's 0x4fb3c0: the screen's lock count, one down.
u32 screenLockCountDown(App* app, ThreadState& ts);
// The game's 0x4fb3d0: the driver's record set up for the screen, and these functions hooked in.
u32 screenInstallDriver(App* app, ThreadState& ts);
// The game's 0x4fb700: the num lock off.
u32 numLockOff(App* app, ThreadState& ts);
// The game's 0x4fb790: a directory's search: a big file's entry opened by its name, 0 when found.
u32 bigFileOpenEntry(App* app, ThreadState& ts, u32 big, u32 search);
// The game's 0x4ead20: the window as it stands set: an error if it is locked.
u32 windowApply(App* app, ThreadState& ts);
// The game's 0x4f7c48: the exit functions registered at 0x9f380c run, last first, once.
u32 exitFunctionsRun(App* app, ThreadState& ts);
// The game's 0x4e0658: nothing, in a table of what can be hooked.
u32 nothingA(App* app, ThreadState& ts);
// The game's 0x4e0659: nothing, in a table of what can be hooked.
u32 nothingB(App* app, ThreadState& ts);
// The game's 0x4f45d0: nothing, in a table of what can be hooked.
u32 nothingC(App* app, ThreadState& ts);
// The game's 0x4f8930: the graphics started: the driver's settings in the environment the first time, the display mode.
u32 graphicsStart(App* app, ThreadState& ts);
// The game's 0x47a970: the sky's shape for a track: its description's words in the shape's name give the sun's direction.
u32 skyLoad(App* app, ThreadState& ts, u32 shapes, u32 shapeName);
// The game's 0x459fe0: an element dropped: where it now is into the layout's table, the HUD rebuilt.
u32 hudEditorDrop(App* app, ThreadState& ts, u32 element, u32 start);
// The game's 0x45a120: an element under the mouse, each frame.
u32 hudEditorDrag(App* app, ThreadState& ts, u32 element);

}  // namespace wata
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
