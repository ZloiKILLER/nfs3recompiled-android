#ifndef NFS3HP_NATIVE_FRONTEND_WATC_H
#define NFS3HP_NATIVE_FRONTEND_WATC_H

#include "native_frontend.h"
#include "native_guest.h"
#include <cstdint>

/* native_frontend_watc.cpp's typed functions (the C runtime's start and end, its locks, threads'
 * data, signals, strtol and the environment, TAPI's stubs, the debug screen's drawing), which other
 * files call as C++ (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `ts` and throws
 * fe::Terminated when the game stopped in one, as a call by address does; its result is what the game's
 * function leaves in eax.  The registers a function reads as it comes in are its parameters
 * (Watcom's order: eax, edx, ebx, ecx, then the stack's words); what it hands on in them to a call or
 * a jump, it puts back there itself.
 *
 * Those that only name something and jump on (the locks' hooks, the drivers' small entries, the
 * hooks that do nothing) are written register by register: they put their arguments in the
 * registers, jump as the code jumps, and return what is left in eax -- with ts.terminate set, not
 * thrown, when the game stopped in it.  A function that takes
 * the address of the words pushed for it (`arguments`: a format's, a pointer handed on) reads them
 * there. */
namespace nfs3hp
{
namespace frontend
{
namespace watc
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;
using s32 = nfs::s32;

// The game's 0x51e110: what a printf that needs floats finds when they are not linked in.
u32 fpNotLoaded(App* app, ThreadState& ts);
// The game's 0x51e2a0: setmbcp(0).
u32 setMbcpZero(App* app, ThreadState& ts);
// The game's 0x51e388: an OS handle in the table of POSIX handles, the table grown to take it, and for 0.
u32 setOsHandle(App* app, ThreadState& ts, u32 osHandle, u32 handle);
// The game's 0x51e51c: the table of handles freed.
u32 freeHandleTable(App* app, ThreadState& ts, u32 eax);
// The game's 0x51e540: a hook that does nothing.
u32 nothing51e540(App* app, ThreadState& ts);
// The game's 0x51f270: a hook that does nothing.
u32 nothing51f270(App* app, ThreadState& ts);
// The game's 0x51e544: a critical section for a lock: one of the 64 kept, then ones allocated.
u32 newSemaphore(App* app, ThreadState& ts);
// The game's 0x51e5e8: the kept critical sections deleted.
u32 deleteKeptSemaphores(App* app, ThreadState& ts, u32 eax);
// The game's 0x51e618: the allocated ones deleted and freed, and their list.
u32 deleteExtraSemaphores(App* app, ThreadState& ts, u32 eax);
// The game's 0x51e670: a lock's owner and count cleared.
u32 clearLock(App* app, ThreadState& ts, u32 lock);
// The game's 0x51e680: the streams' lock taken (sub_51e71c).
u32 lockStreams(App* app, ThreadState& ts);
// The game's 0x51e68c: the streams' lock given back (sub_51e780).
u32 unlockStreams(App* app, ThreadState& ts);
// The game's 0x51e698: a file's lock taken, by its handle.
u32 lockFile(App* app, ThreadState& ts, u32 file);
// The game's 0x51e6a8: a file's lock given back, by its handle.
u32 unlockFile(App* app, ThreadState& ts, u32 file);
// The game's 0x51e6b8: allocPOSIXHandle, jumped to.
u32 allocHandleHook(App* app, ThreadState& ts, u32 osHandle);
// The game's 0x51e6c0: a file's lock cleared and its POSIX handle freed.
u32 freeHandleHook(App* app, ThreadState& ts, u32 handle);
// The game's 0x51e6dc: the heap's lock taken.
u32 lockHeap(App* app, ThreadState& ts);
// The game's 0x51e6e4: the heap's lock given back.
u32 unlockHeap(App* app, ThreadState& ts);
// The game's 0x51e6f0: the file table's lock taken.
u32 lockFileTable(App* app, ThreadState& ts);
// The game's 0x51e6f8: the file table's lock given back.
u32 unlockFileTable(App* app, ThreadState& ts);
// The game's 0x51e704: the thread list's lock taken.
u32 lockThreadList(App* app, ThreadState& ts);
// The game's 0x51e70c: the thread list's lock given back.
u32 unlockThreadList(App* app, ThreadState& ts);
// The game's 0x51e714: the handle table's lock.
u32 lockHandleTable(App* app, ThreadState& ts);
// The game's 0x51e71c: a lock taken: its critical section made the first time, entered unless this thread has it already.
u32 lock(App* app, ThreadState& ts, u32 lock);
// The game's 0x51e778: the handle table's lock given back.
u32 unlockHandleTable(App* app, ThreadState& ts);
// The game's 0x51e780: a lock given back, its critical section left when the count is out.
u32 unlock(App* app, ThreadState& ts, u32 lock);
// The game's 0x51e7a8: the current thread's data, made if it has none and regrown if it asks.
u32 threadData(App* app, ThreadState& ts);
// The game's 0x51e828: the TLS index taken, past 0, 1 and 2 on Windows' older versions.
u32 allocTlsIndex(App* app, ThreadState& ts);
// The game's 0x51e888: a thread's data made, put on the list and in its TLS slot.
u32 attachThread(App* app, ThreadState& ts, u32 block);
// The game's 0x51e8dc: this thread's data off the list and out of its slot; with eax its handle closed.
u32 detachThread(App* app, ThreadState& ts, u32 closeIt);
// The game's 0x51e934: the last thread's end: its data, then the TLS index.
u32 endThreads(App* app, ThreadState& ts);
// The game's 0x51e940: the TLS index given back.
u32 freeTlsIndex(App* app, ThreadState& ts, u32 eax);
// The game's 0x51e964: the runtime made thread-safe, on the first _beginthread: the hooks pointed at the real locks.
u32 initThreads(App* app, ThreadState& ts);
// The game's 0x51ea7c: the runtime's locks taken down at its end.
u32 finiThreads(App* app, ThreadState& ts);
// The game's 0x51ec90: whether the program has an active window, asked of USER32 if it is loadable.
u32 hasActiveWindow(App* app, ThreadState& ts);
// The game's 0x51eccc: a line added to the report, the hex digits after its 0x the value's.
u32 reportAppend(App* app, ThreadState& ts, u32 report, u32 line, u32 value);
// The game's 0x51ed24: the filter that writes the report on stderr, when there is a console to read it.
u32 reportException(App* app, ThreadState& ts, u32 pointers);
// The game's 0x51ef08: the runtime's exception handler: the x87's exceptions and the rest raised as signals.
u32 exceptionHandler(App* app, ThreadState& ts, u32 record, u32 context);
// The game's 0x51f140: the float formatter's hook for printf: the double taken off the arguments, its digits from __LDcvt.
u32 formatFloat(App* app, ThreadState& ts, u32 out, u32 arguments, u32 spec);
// The game's 0x51f254: scanf's hook: strtod, the double stored.
u32 scanFloat(App* app, ThreadState& ts, u32 text, u32 to);
// The game's 0x5236c0: shaperowbytes: a shape's bytes a row, by its type's bits and alignment.
u32 shapeRowBytes(App* app, ThreadState& ts, u32 shape);
// The game's 0x523760: a pixel with its alpha: a 16-bit window blended in place at once.
u32 putPixel(App* app, ThreadState& ts, u32 x, u32 y, u32 colour);
// The game's 0x5239ec: a 4-bit shape's rows through a blending table onto 8-bit pixels.
u32 blend4To8(App* app, ThreadState& ts, u32 arguments, u32 eax);
// The game's 0x523b00: writepixel: a colour already in the window's format, at its depth.
u32 writePixel(App* app, ThreadState& ts, u32 x, u32 y, u32 colour);
// The game's 0x524f88: environ made from Windows' block, the strings copied.
u32 initEnviron(App* app, ThreadState& ts, u32 eax);
// The game's 0x52506c: environ freed, and Windows' block.
u32 freeEnviron(App* app, ThreadState& ts);
// The game's 0x525ba0: setmbcp: the table of lead bytes for a code page.
u32 setMbcp(App* app, ThreadState& ts, u32 asked);
// The game's 0x525e10: a thread that has no data yet: made now.
u32 attachThisThread(App* app, ThreadState& ts);
// The game's 0x525e4c: the thread's data to the size asked now, moved from the block it did not own.
u32 resizeThreadData(App* app, ThreadState& ts);
// The game's 0x525f20: a thread's data on the list: 1, or 0 if there is no memory or the hook says no.
u32 addThread(App* app, ThreadState& ts, u32 thread, u32 data);
// The game's 0x525f84: a thread off the list, its data freed if the list owns it.
u32 removeThread(App* app, ThreadState& ts, u32 thread);
// The game's 0x525ff8: the whole list freed.
u32 freeThreadList(App* app, ThreadState& ts, u32 eax);
// The game's 0x526060: the threads' whole list freed (sub_525ff8), by another name.
u32 freeThreadListJump(App* app, ThreadState& ts, u32 eax);
// The game's 0x526070: the x87's exceptions cleared, which the host has none of.
u32 clearFpExceptions(App* app, ThreadState& ts);
// The game's 0x526080: signal's handler set, the old one returned.
u32 setSignalHandler(App* app, ThreadState& ts, u32 signal, u32 handler);
// The game's 0x5260c8: a signal's handler.
u32 signalHandler(App* app, ThreadState& ts, u32 signal);
// The game's 0x5260ec: the x87's status a signal goes with.
u32 signalMask(App* app, ThreadState& ts, u32 signal);
// The game's 0x526110: the exception handler's question: the handler of a signal that goes with this code.
u32 signalForCode(App* app, ThreadState& ts, u32 signal, u32 code);
// The game's 0x52612c: the console control handler: Ctrl-C and Ctrl-Break as signals.
u32 consoleControl(App* app, ThreadState& ts, u32 event);
// The game's 0x526194: whether the console handler is wanted: not when both signals are ignored or default.
u32 consoleWanted(App* app, ThreadState& ts);
// The game's 0x5261fc: the console handler taken off: 1 when it is.
u32 consoleUnhook(App* app, ThreadState& ts);
// The game's 0x526244: the floating-point signal raised with its code, the handler reset first.
u32 raiseFp(App* app, ThreadState& ts, u32 code);
// The game's 0x526338: raise.
u32 raiseSignal(App* app, ThreadState& ts, u32 signal);
// The game's 0x5263a8: a new thread's handlers, copied from the process's, and the exception handler's hooks set.
u32 initThreadSignals(App* app, ThreadState& ts, u32 ds);
// The game's 0x5263f0: a thread's end: the console signals back to default, when it was wanted.
u32 finiThreadSignals(App* app, ThreadState& ts);
// The game's 0x526420: the two above hooked.
u32 hookThreadSignals(App* app, ThreadState& ts, u32 eax);
// The game's 0x52b523: pow's bad cases: the error reported, or 0 where the answer is 0 with no error; the answer,
// which its code leaves on the x87's stack (eax as it came).
double powError(App* app, ThreadState& ts, u32 arguments);
// The game's 0x52b6c0: a rect's clip, each edge held inside its width and height.
u32 setRectClip(App* app, ThreadState& ts,
                  u32 rect, u32 leftAsked, u32 topAsked, u32 rightAsked, u32 bottomAsked);
// The game's 0x52b720: a box's outline, its corners in any order, as four filled rects.
u32 drawBox(App* app, ThreadState& ts, u32 left, u32 top, u32 right, u32 bottom, u32 colour);
// The game's 0x52b860: a line of text centred, the pen put back.
u32 textCentred(App* app, ThreadState& ts, u32 text, u32 y);
// The game's 0x52b8b0: a rect's clip to all of it.
u32 clipWholeRect(App* app, ThreadState& ts, u32 rect);
// The game's 0x52b8d0: the memory viewer on the window driver's table.
u32 viewerOnWindow(App* app, ThreadState& ts);
// The game's 0x52bb20: the memory viewer on the mono driver's table.
u32 viewerOnMono(App* app, ThreadState& ts);
// The game's 0x52b8e0: the debug window's start.
u32 windowStart(App* app, ThreadState& ts);
// The game's 0x52b8f0: and its end.
u32 windowEnd(App* app, ThreadState& ts);
// The game's 0x52b910: the window driver's sizes: 0 and 1 in characters of 8 pixels, 2 and 3 in pixels.
u32 windowSize(App* app, ThreadState& ts, u32 which);
// The game's 0x52b970: a line printed at a character cell.
u32 printAtCell(App* app, ThreadState& ts, u32 arguments);
// The game's 0x52b9d0: the driver's hook that does nothing.
u32 nothing52b9d0(App* app, ThreadState& ts);
// The game's 0x52b9e0: a shape drawn on black.
u32 blitOnBlack(App* app, ThreadState& ts, u32 shape, u32 x, u32 y);
// The game's 0x52ba00: the text's colours by the debug screen's numbers.
u32 setTextColours(App* app, ThreadState& ts, u32 ink, u32 paper);
// The game's 0x52ba70: a debug-screen colour, one of the CGA's by its number, in the window's format.
u32 debugColour(App* app, ThreadState& ts, u32 colour);
// The game's 0x52bac0: the window driver's key: one waited for, its letter in capitals.
u32 windowWaitKey(App* app, ThreadState& ts);
// The game's 0x52baf0: a filled box in character cells, in a debug colour.
u32 fillCells(App* app, ThreadState& ts, u32 left, u32 top, u32 wide, u32 high, u32 colour);
// The game's 0x52bb50: the mono driver's sizes.
u32 monoSize(App* app, ThreadState& ts, u32 which);
// The game's 0x52bb80: the mono screen cleared.
u32 monoClearJump(App* app, ThreadState& ts, u32 eax);
// The game's 0x52bb90: a shape centred on the cleared screen, on black.
u32 showCentred(App* app, ThreadState& ts, u32 shape);
// The game's 0x52bbf0: the mono screen's attribute, from a debug colour.
u32 monoAttribute(App* app, ThreadState& ts, u32 colour);
// The game's 0x52bc00: a line printed through a window's text at a character cell.
u32 printInWindow(App* app, ThreadState& ts, u32 arguments);
// The game's 0x52bd00: a debug colour's mono attribute.
u32 monoAttributeOf(App* app, ThreadState& ts, u32 colour);
// The game's 0x52bd20: the mono driver's key: as the window's.
u32 monoWaitKey(App* app, ThreadState& ts);
// The game's 0x52bd50: a box of spaces on the mono screen.
u32 monoFill(App* app, ThreadState& ts, u32 left, u32 top, u32 wide, u32 high, u32 colour);
// The game's 0x52bdb0: the handles a parent hands down in C_FILE_INFO: fields handle:os handle:mode*.
u32 inheritHandles(App* app, ThreadState& ts);
// The game's 0x52bed0: environ's own strings freed and its list emptied.
u32 clearEnviron(App* app, ThreadState& ts);
// The game's 0x52c810: 0.
u32 returnZero52c810(App* app, ThreadState& ts);
// The game's 0x52c81c: a call through the thread end's hook.
u32 threadEndHook(App* app, ThreadState& ts, u32 eax);
// The game's 0x52c824: a call through the thread start's hook.
u32 threadStartHook(App* app, ThreadState& ts, u32 eax);
// The game's 0x52ed20: a call through the math error's hook.
u32 mathErrorHook(App* app, ThreadState& ts, u32 eax);
// The game's 0x52c878: abort's report.
u32 abnormalTermination(App* app, ThreadState& ts);
// The game's 0x52c88a: a double's 0 in edx:eax.
std::uint64_t doubleZero(App* app, ThreadState& ts);
// The game's 0x52c89b: the math error, and a double's infinity, in edx:eax, of the sign in what it returns.
std::uint64_t doubleInfinityHook(App* app, ThreadState& ts);
// The game's 0x52c8a7: errno's ERANGE, the math error, and a double's infinity of the sign in eax.
std::uint64_t doubleInfinity(App* app, ThreadState& ts, u32 sign);
// The game's 0x52e3f8: the driver's hook, 0.
u32 zeroJump(App* app, ThreadState& ts);
// The game's 0x52e4e0: a line of text at a position.
u32 textAt(App* app, ThreadState& ts, u32 text, u32 x, u32 y);
// The game's 0x52e500: strtol and strtoul: ecx 1 for the signed.
u32 stringToLong(App* app, ThreadState& ts, u32 text, u32 end, u32 radix, u32 isSigned);
// The game's 0x52e660: strtol: sub_52e500 for the signed.
u32 stringToLongSigned(App* app, ThreadState& ts, u32 text, u32 end, u32 radix);
// The game's 0x52e670: a character's value as a digit of any base, 0x25 for none.
u32 digitValue(App* app, ThreadState& ts, std::uint8_t c);
// The game's 0x52e6d0: putenv: Windows' variable set, then environ, then the wide one if there is one.
u32 putEnvironment(App* app, ThreadState& ts, u32 text);
// The game's 0x52e88c: the string put in environ, or the variable taken out if it has nothing after =.
u32 environSet(App* app, ThreadState& ts, u32 text, u32 edx);
// The game's 0x52ea3c: a variable looked for in environ, case aside: its index + 1.
u32 environFind(App* app, ThreadState& ts, u32 name, u32 removing);
// The game's 0x52eb70: a thread's first function: its data made, _beginthread told, the function called, the thread's end.
u32 threadStart(App* app, ThreadState& ts, u32 parameters);
// The game's 0x52ec04: _beginthread: the thread made and waited for until it has its data.
u32 beginThread(App* app, ThreadState& ts, u32 function, u32 stackSize, u32 argument);
// The game's 0x52ecf0: _endthread.
u32 endThread(App* app, ThreadState& ts);
// The game's 0x52edfa: 0.
u32 returnZero52edfa(App* app, ThreadState& ts);
// The game's 0x52ee00: the window's clip, held inside the screen.
u32 setClip(App* app, ThreadState& ts, u32 leftAsked, u32 topAsked, u32 rightAsked, u32 bottomAsked);
// The game's 0x52ee60: _mbschr: the multibyte character found, or the end if asked for 0.
u32 mbschr(App* app, ThreadState& ts, u32 text, u32 wanted);
// The game's 0x52f318: TAPI32.DLL loaded if it is not, a function looked up into its slot.
u32 tapiFind(App* app, ThreadState& ts, u32 name, u32 slot);
// The game's 0x52f535: lineAnswer.
u32 lineAnswer(App* app, ThreadState& ts, u32 callHandle, u32 userUserInfo, u32 size);
// The game's 0x52f661: lineClose.
u32 lineClose(App* app, ThreadState& ts, u32 line);
// The game's 0x52f969: lineDeallocateCall.
u32 lineDeallocateCall(App* app, ThreadState& ts, u32 callHandle);
// The game's 0x52fb2e: lineDrop.
u32 lineDrop(App* app, ThreadState& ts, u32 callHandle, u32 userUserInfo, u32 size);
// The game's 0x530747: lineGetDevCaps.
u32 lineGetDevCaps(App* app, ThreadState& ts, u32 lineApp, u32 device, u32 apiVersion, u32 extVersion, u32 caps);
// The game's 0x53083d: lineGetDevConfig.
u32 lineGetDevConfig(App* app, ThreadState& ts, u32 device, u32 config, u32 deviceClass);
// The game's 0x530a4e: lineGetID.
u32 lineGetID(App* app, ThreadState& ts,
                u32 line, u32 address, u32 callHandle, u32 select, u32 deviceId, u32 deviceClass);
// The game's 0x530b4d: lineGetLineDevStatus.
u32 lineGetLineDevStatus(App* app, ThreadState& ts, u32 line, u32 status);
// The game's 0x5310be: lineInitialize.
u32 lineInitialize(App* app, ThreadState& ts,
                     u32 lineAppOut, u32 instance, u32 callback, u32 appName, u32 devicesOut);
// The game's 0x5311c0: lineMakeCall.
u32 lineMakeCall(App* app, ThreadState& ts, u32 line, u32 callOut, u32 destination, u32 country, u32 callParams);
// The game's 0x53138f: lineNegotiateAPIVersion.
u32 lineNegotiateAPIVersion(App* app, ThreadState& ts,
                              u32 lineApp, u32 device, u32 lowVersion, u32 highVersion, u32 versionOut, u32 extensionId);
// The game's 0x531439: lineOpen.
u32 lineOpen(App* app, ThreadState& ts,
               u32 lineApp, u32 device, u32 lineOut, u32 apiVersion, u32 extVersion, u32 callbackInstance, u32 privileges, u32 mediaModes, u32 callParams);
// The game's 0x531f31: lineSetDevConfig.
u32 lineSetDevConfig(App* app, ThreadState& ts, u32 device, u32 config, u32 size, u32 deviceClass);
// The game's 0x532162: lineSetStatusMessages.
u32 lineSetStatusMessages(App* app, ThreadState& ts, u32 line, u32 lineStates, u32 addressStates);
// The game's 0x5324d2: lineShutdown.
u32 lineShutdown(App* app, ThreadState& ts, u32 lineApp);

}  // namespace watc
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
