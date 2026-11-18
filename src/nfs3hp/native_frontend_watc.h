#ifndef NFS3HP_NATIVE_FRONTEND_WATC_H
#define NFS3HP_NATIVE_FRONTEND_WATC_H

#include "native_frontend.h"
#include "native_guest.h"
#include <cstdint>

/* native_frontend_watc.cpp's typed functions (the C runtime's start and end, its locks, threads'
 * data, signals, strtol and the environment, TAPI's stubs, the debug screen's drawing), which other
 * files call as C++ (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `cpu` and throws
 * fe::Terminated when the game stopped in one, as a call by address does; its result is what the game's
 * function leaves in eax.  The registers a function reads as it comes in are its parameters
 * (Watcom's order: eax, edx, ebx, ecx, then the stack's words); what it hands on in them to a call or
 * a jump, it puts back there itself.
 *
 * Those that only name something and jump on (the locks' hooks, the drivers' small entries, the
 * hooks that do nothing) are written register by register: they put their arguments in the
 * registers, jump as the code jumps, and return what is left in eax -- with cpu.terminate set, not
 * thrown, when the game stopped in it.  A function that takes
 * the address of the words pushed for it (`arguments`: a format's, a pointer handed on) reads them
 * there. */
namespace nfs3hp
{
namespace frontend
{
namespace watc
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;
using sreg32 = x86::sreg32;

// The game's 0x51e110: what a printf that needs floats finds when they are not linked in.
reg32 fpNotLoaded(App* app, CPU& cpu);
// The game's 0x51e2a0: setmbcp(0).
reg32 setMbcpZero(App* app, CPU& cpu);
// The game's 0x51e388: an OS handle in the table of POSIX handles, the table grown to take it, and for 0.
reg32 setOsHandle(App* app, CPU& cpu, reg32 osHandle, reg32 handle);
// The game's 0x51e51c: the table of handles freed.
reg32 freeHandleTable(App* app, CPU& cpu, reg32 eax);
// The game's 0x51e540: a hook that does nothing.
reg32 nothing51e540(App* app, CPU& cpu);
// The game's 0x51f270: a hook that does nothing.
reg32 nothing51f270(App* app, CPU& cpu);
// The game's 0x51e544: a critical section for a lock: one of the 64 kept, then ones allocated.
reg32 newSemaphore(App* app, CPU& cpu);
// The game's 0x51e5e8: the kept critical sections deleted.
reg32 deleteKeptSemaphores(App* app, CPU& cpu, reg32 eax);
// The game's 0x51e618: the allocated ones deleted and freed, and their list.
reg32 deleteExtraSemaphores(App* app, CPU& cpu, reg32 eax);
// The game's 0x51e670: a lock's owner and count cleared.
reg32 clearLock(App* app, CPU& cpu, reg32 lock);
// The game's 0x51e680: the streams' lock taken (sub_51e71c).
reg32 lockStreams(App* app, CPU& cpu);
// The game's 0x51e68c: the streams' lock given back (sub_51e780).
reg32 unlockStreams(App* app, CPU& cpu);
// The game's 0x51e698: a file's lock taken, by its handle.
reg32 lockFile(App* app, CPU& cpu, reg32 file);
// The game's 0x51e6a8: a file's lock given back, by its handle.
reg32 unlockFile(App* app, CPU& cpu, reg32 file);
// The game's 0x51e6b8: allocPOSIXHandle, jumped to.
reg32 allocHandleHook(App* app, CPU& cpu, reg32 osHandle);
// The game's 0x51e6c0: a file's lock cleared and its POSIX handle freed.
reg32 freeHandleHook(App* app, CPU& cpu, reg32 handle);
// The game's 0x51e6dc: the heap's lock taken.
reg32 lockHeap(App* app, CPU& cpu);
// The game's 0x51e6e4: the heap's lock given back.
reg32 unlockHeap(App* app, CPU& cpu);
// The game's 0x51e6f0: the file table's lock taken.
reg32 lockFileTable(App* app, CPU& cpu);
// The game's 0x51e6f8: the file table's lock given back.
reg32 unlockFileTable(App* app, CPU& cpu);
// The game's 0x51e704: the thread list's lock taken.
reg32 lockThreadList(App* app, CPU& cpu);
// The game's 0x51e70c: the thread list's lock given back.
reg32 unlockThreadList(App* app, CPU& cpu);
// The game's 0x51e714: the handle table's lock.
reg32 lockHandleTable(App* app, CPU& cpu);
// The game's 0x51e71c: a lock taken: its critical section made the first time, entered unless this thread has it already.
reg32 lock(App* app, CPU& cpu, reg32 lock);
// The game's 0x51e778: the handle table's lock given back.
reg32 unlockHandleTable(App* app, CPU& cpu);
// The game's 0x51e780: a lock given back, its critical section left when the count is out.
reg32 unlock(App* app, CPU& cpu, reg32 lock);
// The game's 0x51e7a8: the current thread's data, made if it has none and regrown if it asks.
reg32 threadData(App* app, CPU& cpu);
// The game's 0x51e828: the TLS index taken, past 0, 1 and 2 on Windows' older versions.
reg32 allocTlsIndex(App* app, CPU& cpu);
// The game's 0x51e888: a thread's data made, put on the list and in its TLS slot.
reg32 attachThread(App* app, CPU& cpu, reg32 block);
// The game's 0x51e8dc: this thread's data off the list and out of its slot; with eax its handle closed.
reg32 detachThread(App* app, CPU& cpu, reg32 closeIt);
// The game's 0x51e934: the last thread's end: its data, then the TLS index.
reg32 endThreads(App* app, CPU& cpu);
// The game's 0x51e940: the TLS index given back.
reg32 freeTlsIndex(App* app, CPU& cpu, reg32 eax);
// The game's 0x51e964: the runtime made thread-safe, on the first _beginthread: the hooks pointed at the real locks.
reg32 initThreads(App* app, CPU& cpu);
// The game's 0x51ea7c: the runtime's locks taken down at its end.
reg32 finiThreads(App* app, CPU& cpu);
// The game's 0x51ec90: whether the program has an active window, asked of USER32 if it is loadable.
reg32 hasActiveWindow(App* app, CPU& cpu);
// The game's 0x51eccc: a line added to the report, the hex digits after its 0x the value's.
reg32 reportAppend(App* app, CPU& cpu, reg32 report, reg32 line, reg32 value);
// The game's 0x51ed24: the filter that writes the report on stderr, when there is a console to read it.
reg32 reportException(App* app, CPU& cpu, reg32 pointers);
// The game's 0x51ef08: the runtime's exception handler: the x87's exceptions and the rest raised as signals.
reg32 exceptionHandler(App* app, CPU& cpu, reg32 record, reg32 context);
// The game's 0x51f140: the float formatter's hook for printf: the double taken off the arguments, its digits from __LDcvt.
reg32 formatFloat(App* app, CPU& cpu, reg32 out, reg32 arguments, reg32 spec);
// The game's 0x51f254: scanf's hook: strtod, the double stored.
reg32 scanFloat(App* app, CPU& cpu, reg32 text, reg32 to);
// The game's 0x5236c0: shaperowbytes: a shape's bytes a row, by its type's bits and alignment.
reg32 shapeRowBytes(App* app, CPU& cpu, reg32 shape);
// The game's 0x523760: a pixel with its alpha: a 16-bit window blended in place at once.
reg32 putPixel(App* app, CPU& cpu, reg32 x, reg32 y, reg32 colour);
// The game's 0x5239ec: a 4-bit shape's rows through a blending table onto 8-bit pixels.
reg32 blend4To8(App* app, CPU& cpu, reg32 arguments, reg32 eax);
// The game's 0x523b00: writepixel: a colour already in the window's format, at its depth.
reg32 writePixel(App* app, CPU& cpu, reg32 x, reg32 y, reg32 colour);
// The game's 0x524f88: environ made from Windows' block, the strings copied.
reg32 initEnviron(App* app, CPU& cpu, reg32 eax);
// The game's 0x52506c: environ freed, and Windows' block.
reg32 freeEnviron(App* app, CPU& cpu);
// The game's 0x525ba0: setmbcp: the table of lead bytes for a code page.
reg32 setMbcp(App* app, CPU& cpu, reg32 asked);
// The game's 0x525e10: a thread that has no data yet: made now.
reg32 attachThisThread(App* app, CPU& cpu);
// The game's 0x525e4c: the thread's data to the size asked now, moved from the block it did not own.
reg32 resizeThreadData(App* app, CPU& cpu);
// The game's 0x525f20: a thread's data on the list: 1, or 0 if there is no memory or the hook says no.
reg32 addThread(App* app, CPU& cpu, reg32 thread, reg32 data);
// The game's 0x525f84: a thread off the list, its data freed if the list owns it.
reg32 removeThread(App* app, CPU& cpu, reg32 thread);
// The game's 0x525ff8: the whole list freed.
reg32 freeThreadList(App* app, CPU& cpu, reg32 eax);
// The game's 0x526060: the threads' whole list freed (sub_525ff8), by another name.
reg32 freeThreadListJump(App* app, CPU& cpu, reg32 eax);
// The game's 0x526070: the x87's exceptions cleared, which the host has none of.
reg32 clearFpExceptions(App* app, CPU& cpu);
// The game's 0x526080: signal's handler set, the old one returned.
reg32 setSignalHandler(App* app, CPU& cpu, reg32 signal, reg32 handler);
// The game's 0x5260c8: a signal's handler.
reg32 signalHandler(App* app, CPU& cpu, reg32 signal);
// The game's 0x5260ec: the x87's status a signal goes with.
reg32 signalMask(App* app, CPU& cpu, reg32 signal);
// The game's 0x526110: the exception handler's question: the handler of a signal that goes with this code.
reg32 signalForCode(App* app, CPU& cpu, reg32 signal, reg32 code);
// The game's 0x52612c: the console control handler: Ctrl-C and Ctrl-Break as signals.
reg32 consoleControl(App* app, CPU& cpu, reg32 event);
// The game's 0x526194: whether the console handler is wanted: not when both signals are ignored or default.
reg32 consoleWanted(App* app, CPU& cpu);
// The game's 0x5261fc: the console handler taken off: 1 when it is.
reg32 consoleUnhook(App* app, CPU& cpu);
// The game's 0x526244: the floating-point signal raised with its code, the handler reset first.
reg32 raiseFp(App* app, CPU& cpu, reg32 code);
// The game's 0x526338: raise.
reg32 raiseSignal(App* app, CPU& cpu, reg32 signal);
// The game's 0x5263a8: a new thread's handlers, copied from the process's, and the exception handler's hooks set.
reg32 initThreadSignals(App* app, CPU& cpu, reg32 ds);
// The game's 0x5263f0: a thread's end: the console signals back to default, when it was wanted.
reg32 finiThreadSignals(App* app, CPU& cpu);
// The game's 0x526420: the two above hooked.
reg32 hookThreadSignals(App* app, CPU& cpu, reg32 eax);
// The game's 0x52b523: pow's bad cases: the error reported, or 0 where the answer is 0 with no error; the answer,
// which its code leaves on the x87's stack (eax as it came).
double powError(App* app, CPU& cpu, reg32 arguments);
// The game's 0x52b6c0: a rect's clip, each edge held inside its width and height.
reg32 setRectClip(App* app, CPU& cpu,
                  reg32 rect, reg32 leftAsked, reg32 topAsked, reg32 rightAsked, reg32 bottomAsked);
// The game's 0x52b720: a box's outline, its corners in any order, as four filled rects.
reg32 drawBox(App* app, CPU& cpu, reg32 left, reg32 top, reg32 right, reg32 bottom, reg32 colour);
// The game's 0x52b860: a line of text centred, the pen put back.
reg32 textCentred(App* app, CPU& cpu, reg32 text, reg32 y);
// The game's 0x52b8b0: a rect's clip to all of it.
reg32 clipWholeRect(App* app, CPU& cpu, reg32 rect);
// The game's 0x52b8d0: the memory viewer on the window driver's table.
reg32 viewerOnWindow(App* app, CPU& cpu);
// The game's 0x52bb20: the memory viewer on the mono driver's table.
reg32 viewerOnMono(App* app, CPU& cpu);
// The game's 0x52b8e0: the debug window's start.
reg32 windowStart(App* app, CPU& cpu);
// The game's 0x52b8f0: and its end.
reg32 windowEnd(App* app, CPU& cpu);
// The game's 0x52b910: the window driver's sizes: 0 and 1 in characters of 8 pixels, 2 and 3 in pixels.
reg32 windowSize(App* app, CPU& cpu, reg32 which);
// The game's 0x52b970: a line printed at a character cell.
reg32 printAtCell(App* app, CPU& cpu, reg32 arguments);
// The game's 0x52b9d0: the driver's hook that does nothing.
reg32 nothing52b9d0(App* app, CPU& cpu);
// The game's 0x52b9e0: a shape drawn on black.
reg32 blitOnBlack(App* app, CPU& cpu, reg32 shape, reg32 x, reg32 y);
// The game's 0x52ba00: the text's colours by the debug screen's numbers.
reg32 setTextColours(App* app, CPU& cpu, reg32 ink, reg32 paper);
// The game's 0x52ba70: a debug-screen colour, one of the CGA's by its number, in the window's format.
reg32 debugColour(App* app, CPU& cpu, reg32 colour);
// The game's 0x52bac0: the window driver's key: one waited for, its letter in capitals.
reg32 windowWaitKey(App* app, CPU& cpu);
// The game's 0x52baf0: a filled box in character cells, in a debug colour.
reg32 fillCells(App* app, CPU& cpu, reg32 left, reg32 top, reg32 wide, reg32 high, reg32 colour);
// The game's 0x52bb50: the mono driver's sizes.
reg32 monoSize(App* app, CPU& cpu, reg32 which);
// The game's 0x52bb80: the mono screen cleared.
reg32 monoClearJump(App* app, CPU& cpu, reg32 eax);
// The game's 0x52bb90: a shape centred on the cleared screen, on black.
reg32 showCentred(App* app, CPU& cpu, reg32 shape);
// The game's 0x52bbf0: the mono screen's attribute, from a debug colour.
reg32 monoAttribute(App* app, CPU& cpu, reg32 colour);
// The game's 0x52bc00: a line printed through a window's text at a character cell.
reg32 printInWindow(App* app, CPU& cpu, reg32 arguments);
// The game's 0x52bd00: a debug colour's mono attribute.
reg32 monoAttributeOf(App* app, CPU& cpu, reg32 colour);
// The game's 0x52bd20: the mono driver's key: as the window's.
reg32 monoWaitKey(App* app, CPU& cpu);
// The game's 0x52bd50: a box of spaces on the mono screen.
reg32 monoFill(App* app, CPU& cpu, reg32 left, reg32 top, reg32 wide, reg32 high, reg32 colour);
// The game's 0x52bdb0: the handles a parent hands down in C_FILE_INFO: fields handle:os handle:mode*.
reg32 inheritHandles(App* app, CPU& cpu);
// The game's 0x52bed0: environ's own strings freed and its list emptied.
reg32 clearEnviron(App* app, CPU& cpu);
// The game's 0x52c810: 0.
reg32 returnZero52c810(App* app, CPU& cpu);
// The game's 0x52c81c: a call through the thread end's hook.
reg32 threadEndHook(App* app, CPU& cpu, reg32 eax);
// The game's 0x52c824: a call through the thread start's hook.
reg32 threadStartHook(App* app, CPU& cpu, reg32 eax);
// The game's 0x52ed20: a call through the math error's hook.
reg32 mathErrorHook(App* app, CPU& cpu, reg32 eax);
// The game's 0x52c878: abort's report.
reg32 abnormalTermination(App* app, CPU& cpu);
// The game's 0x52c88a: a double's 0 in edx:eax.
std::uint64_t doubleZero(App* app, CPU& cpu);
// The game's 0x52c89b: the math error, and a double's infinity, in edx:eax, of the sign in what it returns.
std::uint64_t doubleInfinityHook(App* app, CPU& cpu);
// The game's 0x52c8a7: errno's ERANGE, the math error, and a double's infinity of the sign in eax.
std::uint64_t doubleInfinity(App* app, CPU& cpu, reg32 sign);
// The game's 0x52e3f8: the driver's hook, 0.
reg32 zeroJump(App* app, CPU& cpu);
// The game's 0x52e4e0: a line of text at a position.
reg32 textAt(App* app, CPU& cpu, reg32 text, reg32 x, reg32 y);
// The game's 0x52e500: strtol and strtoul: ecx 1 for the signed.
reg32 stringToLong(App* app, CPU& cpu, reg32 text, reg32 end, reg32 radix, reg32 isSigned);
// The game's 0x52e660: strtol: sub_52e500 for the signed.
reg32 stringToLongSigned(App* app, CPU& cpu, reg32 text, reg32 end, reg32 radix);
// The game's 0x52e670: a character's value as a digit of any base, 0x25 for none.
reg32 digitValue(App* app, CPU& cpu, std::uint8_t c);
// The game's 0x52e6d0: putenv: Windows' variable set, then environ, then the wide one if there is one.
reg32 putEnvironment(App* app, CPU& cpu, reg32 text);
// The game's 0x52e88c: the string put in environ, or the variable taken out if it has nothing after =.
reg32 environSet(App* app, CPU& cpu, reg32 text, reg32 edx);
// The game's 0x52ea3c: a variable looked for in environ, case aside: its index + 1.
reg32 environFind(App* app, CPU& cpu, reg32 name, reg32 removing);
// The game's 0x52eb70: a thread's first function: its data made, _beginthread told, the function called, the thread's end.
reg32 threadStart(App* app, CPU& cpu, reg32 parameters);
// The game's 0x52ec04: _beginthread: the thread made and waited for until it has its data.
reg32 beginThread(App* app, CPU& cpu, reg32 function, reg32 stackSize, reg32 argument);
// The game's 0x52ecf0: _endthread.
reg32 endThread(App* app, CPU& cpu);
// The game's 0x52edfa: 0.
reg32 returnZero52edfa(App* app, CPU& cpu);
// The game's 0x52ee00: the window's clip, held inside the screen.
reg32 setClip(App* app, CPU& cpu, reg32 leftAsked, reg32 topAsked, reg32 rightAsked, reg32 bottomAsked);
// The game's 0x52ee60: _mbschr: the multibyte character found, or the end if asked for 0.
reg32 mbschr(App* app, CPU& cpu, reg32 text, reg32 wanted);
// The game's 0x52f318: TAPI32.DLL loaded if it is not, a function looked up into its slot.
reg32 tapiFind(App* app, CPU& cpu, reg32 name, reg32 slot);
// The game's 0x52f535: lineAnswer.
reg32 lineAnswer(App* app, CPU& cpu, reg32 callHandle, reg32 userUserInfo, reg32 size);
// The game's 0x52f661: lineClose.
reg32 lineClose(App* app, CPU& cpu, reg32 line);
// The game's 0x52f969: lineDeallocateCall.
reg32 lineDeallocateCall(App* app, CPU& cpu, reg32 callHandle);
// The game's 0x52fb2e: lineDrop.
reg32 lineDrop(App* app, CPU& cpu, reg32 callHandle, reg32 userUserInfo, reg32 size);
// The game's 0x530747: lineGetDevCaps.
reg32 lineGetDevCaps(App* app, CPU& cpu, reg32 lineApp, reg32 device, reg32 apiVersion, reg32 extVersion, reg32 caps);
// The game's 0x53083d: lineGetDevConfig.
reg32 lineGetDevConfig(App* app, CPU& cpu, reg32 device, reg32 config, reg32 deviceClass);
// The game's 0x530a4e: lineGetID.
reg32 lineGetID(App* app, CPU& cpu,
                reg32 line, reg32 address, reg32 callHandle, reg32 select, reg32 deviceId, reg32 deviceClass);
// The game's 0x530b4d: lineGetLineDevStatus.
reg32 lineGetLineDevStatus(App* app, CPU& cpu, reg32 line, reg32 status);
// The game's 0x5310be: lineInitialize.
reg32 lineInitialize(App* app, CPU& cpu,
                     reg32 lineAppOut, reg32 instance, reg32 callback, reg32 appName, reg32 devicesOut);
// The game's 0x5311c0: lineMakeCall.
reg32 lineMakeCall(App* app, CPU& cpu, reg32 line, reg32 callOut, reg32 destination, reg32 country, reg32 callParams);
// The game's 0x53138f: lineNegotiateAPIVersion.
reg32 lineNegotiateAPIVersion(App* app, CPU& cpu,
                              reg32 lineApp, reg32 device, reg32 lowVersion, reg32 highVersion, reg32 versionOut, reg32 extensionId);
// The game's 0x531439: lineOpen.
reg32 lineOpen(App* app, CPU& cpu,
               reg32 lineApp, reg32 device, reg32 lineOut, reg32 apiVersion, reg32 extVersion, reg32 callbackInstance, reg32 privileges, reg32 mediaModes, reg32 callParams);
// The game's 0x531f31: lineSetDevConfig.
reg32 lineSetDevConfig(App* app, CPU& cpu, reg32 device, reg32 config, reg32 size, reg32 deviceClass);
// The game's 0x532162: lineSetStatusMessages.
reg32 lineSetStatusMessages(App* app, CPU& cpu, reg32 line, reg32 lineStates, reg32 addressStates);
// The game's 0x5324d2: lineShutdown.
reg32 lineShutdown(App* app, CPU& cpu, reg32 lineApp);

}  // namespace watc
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
