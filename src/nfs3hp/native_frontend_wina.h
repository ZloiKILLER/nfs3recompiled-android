#ifndef NFS3HP_NATIVE_FRONTEND_WINA_H
#define NFS3HP_NATIVE_FRONTEND_WINA_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_wina.cpp's typed functions (the buffered stream under the movie player, EA's file
 * library's operations, the game's window and its messages, the startup checks, what is left of the
 * graphics library, the palette and the runtime), which other files call as C++ (guest::direct),
 * and how the game's code calls each by address (ByAddress, for the checks' stand-ins).  Each calls
 * the game's other functions by address on `cpu` and throws fe::Terminated when the game stopped in
 * one, as Guest::call does; its result is what the game's function leaves in eax.  A register past
 * the parameters a function takes is handed on as the caller has it. */
namespace nfs3hp
{
namespace frontend
{
namespace wina
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x4cf1c0: a handle to its stream, both handed out; nonzero when it is none.
reg32 streamCheck(App* app, CPU& cpu, reg32 handle, reg32 streamOut, reg32 handleOut);
// The game's 0x4cf1f0: whether a place of the ring is between two others, the way round.
reg32 ringBetween(App* app, CPU& cpu, reg32 from, reg32 to, reg32 place);
// The game's 0x4cf220: bytes taken off what is queued; below the low water, more is asked for sooner: the read in
// hand gets the busy priority.
reg32 streamConsume(App* app, CPU& cpu, reg32 s, reg32 amount);
// The game's 0x4cf280: a file record off the free list, its id's serial moved on; 0 when none.
reg32 streamTakeFree(App* app, CPU& cpu, reg32 s);
// The game's 0x4cf2f0: a file record on the end of the queue.
reg32 streamQueueFile(App* app, CPU& cpu, reg32 s, reg32 record);
// The game's 0x4cf340: the file record of an id, 0 when it has gone.
reg32 streamFindFile(App* app, CPU& cpu, reg32 s, reg32 id);
// The game's 0x4cf380: a file record off the queue, onto the free list.
reg32 streamUnlinkFile(App* app, CPU& cpu, reg32 s, reg32 record);
// The game's 0x4cf3f0: the channel a chunk's id goes to, by the rules (mask, value, channel); -2 none.
reg32 streamClassify(App* app, CPU& cpu, reg32 s, reg32 chunk);
// The game's 0x4cf440: the whole chunks read so far given to their channels; 1 when the current file's last chunk
// has come.
reg32 streamParse(App* app, CPU& cpu, reg32 s);
// The game's 0x4cf570: the file's open has ended: its handle kept, reading begins.
reg32 streamFileOpened(App* app, CPU& cpu, reg32 s);
// The game's 0x4cf5a0: the last file's close has ended: the next one opened.
reg32 streamFileSwitched(App* app, CPU& cpu, reg32 s);
// The game's 0x4cf5e0: a read has ended, or a memory block was copied: the chunks parsed, and on to more of this
// file or the next.
reg32 streamFilled(App* app, CPU& cpu, reg32 s);
// The game's 0x4cf6b0: on to the next file of the queue: from memory at once, from a file by an open, after a
// close when another one is open.
reg32 streamNextFile(App* app, CPU& cpu, reg32 s, reg32 priority);
// The game's 0x4cf810: the next read into the ring: records of files read through let go, the ring wrapped when
// the end is too near, and 0x2000 bytes asked for or copied.
reg32 streamReadMore(App* app, CPU& cpu, reg32 s, reg32 priority);
// The game's 0x4cfa40: the bytes a stream's record and tables take.
reg32 streamHeaderSize(App* app, CPU& cpu, reg32 fileCount, reg32 ruleCount, reg32 channelCount);
// The game's 0x4cfa80: a stream made in the memory given; its first channel, or 0 when it does not fit or asks too
// much.
reg32 streamCreate(App* app, CPU& cpu, reg32 files, reg32 rules, reg32 channels, reg32 s, reg32 memory);
// The game's 0x4cfca0: a rule of a stream that is not reading: mask, value, channel.
reg32 streamSetRule(App* app, CPU& cpu, reg32 handle, reg32 number, reg32 mask, reg32 value, reg32 target);
// The game's 0x4cfd30: a stream closed: flushed, its read waited for, its lock and file gone.
reg32 streamClose(App* app, CPU& cpu, reg32 handle);
// The game's 0x4cfdd0: the low water mark: crossing it tells the read in hand.
reg32 streamSetLowWater(App* app, CPU& cpu, reg32 handle, reg32 low);
// The game's 0x4cfe30: whether the stream is below its low water; when it is, the read gets the busy priority.
reg32 streamSetBelow(App* app, CPU& cpu, reg32 handle, reg32 below);
// The game's 0x4cfe70: a channel's handle, by its number from 1; 0 when there is none.
reg32 streamChannel(App* app, CPU& cpu, reg32 handle, reg32 number);
// The game's 0x4cfef0: a file put on the queue; reading starts if it had stopped.  Its id, 0 when there is no
// room.
reg32 streamOpenFile(App* app, CPU& cpu, reg32 handle, reg32 name, reg32 length, reg32 endId);
// The game's 0x4cffb0: a block of memory put on the queue; with no length given, it runs to the chunk with the end
// id.  Its id, 0 when there is no room.
reg32 streamOpenMemory(App* app, CPU& cpu, reg32 handle, reg32 data, reg32 length, reg32 endId);
// The game's 0x4d0060: a file of the queue closed: what it has in the ring taken back from every channel, by
// freeing its chunks or marking them.
reg32 streamCloseFile(App* app, CPU& cpu, reg32 handle, reg32 id);
// The game's 0x4d0220: everything off a stream: its files closed, the channels emptied, the ring marked free.
reg32 streamFlush(App* app, CPU& cpu, reg32 handle);
// The game's 0x4d0300: a channel's next chunk, its reader moved on past it; 0 when it has none.
reg32 streamNextChunk(App* app, CPU& cpu, reg32 handle);
// The game's 0x4d03c0: a chunk read and done with: if reading waited for room, on it goes.
reg32 streamFreeChunk(App* app, CPU& cpu, reg32 handle, reg32 chunk);
// The game's 0x4d0460: the bytes a channel has waiting.
reg32 channelBytes(App* app, CPU& cpu, reg32 handle);
// The game's 0x4d0490: the state of a channel's stream.
reg32 channelState(App* app, CPU& cpu, reg32 handle);
// The game's 0x4d04c0: whether a channel has come to its end: nothing waiting, nothing reading.
reg32 channelDone(App* app, CPU& cpu, reg32 handle);
// The game's 0x4d0500: how big a channel's stream's ring is.
reg32 channelRingSize(App* app, CPU& cpu, reg32 handle);
// The game's 0x495b60: the movie's next frame chunk from its channel; what is not one is thrown away.  0 at the
// channel's end.  It lets other threads run at every try.
reg32 movieNextFrame(App* app, CPU& cpu, reg32 reader);
// The game's 0x4fb990: a file operation off the free list, set up; its slot's serial in its id.
reg32 fileOpAllocate(App* app, CPU& cpu, reg32 kind, reg32 priorityIn, reg32 target, reg32 slot);
// The game's 0x4fbb60: an operation on its slot's queue, the slot's thread told; its id.
reg32 fileOpQueue(App* app, CPU& cpu, reg32 op);
// The game's 0x4fbbe0: an operation's file name, a copy of it in memory of its own.
reg32 fileOpSetName(App* app, CPU& cpu, reg32 op, reg32 nameIn);
// The game's 0x4fc8e0: what is to be called when an operation ends; at once when it has.
reg32 fileOpCallback(App* app, CPU& cpu, reg32 id, reg32 callback);
// The game's 0x4fc9b0: an operation still queued given another priority.
reg32 fileOpPriority(App* app, CPU& cpu, reg32 id, reg32 priority);
// The game's 0x4fca60: an operation to open a file.
reg32 fileOpOpen(App* app, CPU& cpu, reg32 name, reg32 mode, reg32 priority, reg32 target);
// The game's 0x4fca90: an operation to close a file.
reg32 fileOpClose(App* app, CPU& cpu, reg32 handle, reg32 priority, reg32 target);
// The game's 0x4fcab0: an operation to read a file.
reg32 fileOpRead(App* app, CPU& cpu, reg32 handle, reg32 offset, reg32 buffer, reg32 size, reg32 priority,
                 reg32 target);
// The game's 0x4fce50: the slot of a file's name: 0 when it has a '|', else by its drive.
reg32 fileSlotOfName(App* app, CPU& cpu, reg32 name);
// The game's 0x4fd060: a file closed and waited for.
reg32 fileCloseWait(App* app, CPU& cpu, reg32 handle, reg32 priority);
// The game's 0x4e4230: a file's size, 0 when the handle is not of an open one or it keeps no system handle; the
// file pointer put where asked.  Under the table's own lock.
reg32 fileSizeAndSeek(App* app, CPU& cpu, reg32 handle, reg32 offset);
// The game's 0x4e4320: which lock of the table an open file's handle has; 0 when none.
reg32 fileLockOf(App* app, CPU& cpu, reg32 handle);
// The game's 0x4cc650: two handler rows, by their message: for qsort and bsearch.
reg32 compareHandlers(App* app, CPU& cpu, reg32 leftRow, reg32 rightRow);
// The game's 0x4cc660: a message's handler, 0 when it has none.
reg32 handlerOf(App* app, CPU& cpu, reg32 message);
// The game's 0x4cc6d0: a message's handler set, changed, or taken away with 0; the rows stay sorted.  1 when done,
// 0 when the table is full or it had none to take away.
reg32 handlerSet(App* app, CPU& cpu, reg32 message, reg32 handler);
// The game's 0x4cc7c0: a message to the game's window, sent or posted; 1 when it went.
reg32 postMessage(App* app, CPU& cpu, reg32 message, reg32 lParam, reg32 wParam, reg32 send);
// The game's 0x4cc860: who is told when the window gets or loses the focus.
reg32 setFocusCallback(App* app, CPU& cpu, reg32 callback);
// The game's 0x4cc870: the system task that leaves the game when there is no window.
reg32 noWindowTask(App* app, CPU& cpu);
// The game's 0x4cc890: the system's cursor put at a place of the window.
reg32 movePointer(App* app, CPU& cpu, reg32 window, reg32 x, reg32 y);
// The game's 0x4cc8e0: a key on the queue, the oldest dropped when it is full.
reg32 pushKey(App* app, CPU& cpu, reg32 key, reg32 scan, reg32 flags);
// The game's 0x4ccaa0: the key code of the arrows and the function keys, from a key-down message's virtual key; 0
// for anything else.  The generated code's jumps are one table.
reg32 keyOfMessage(App* app, CPU& cpu, reg32 message, reg32 virtualKey);
// The game's 0x4ccc70: the low-level keyboard hook: Alt+Tab, Alt+Esc, Ctrl+Esc and the Windows keys told to the
// game instead of the system.
reg32 keyboardHook(App* app, CPU& cpu, reg32 code, reg32 wParam, reg32 lParam);
// The game's 0x4ccd60: the window has the focus: hook and callback.
reg32 onSetFocus(App* app, CPU& cpu, reg32 window, reg32 hwnd, reg32 message, reg32 wParam, reg32 lParam, reg32 result);
// The game's 0x4ccdd0: the window has lost the focus: the hook off, the keys let go.
reg32 onKillFocus(App* app, CPU& cpu, reg32 window, reg32 hwnd, reg32 message, reg32 wParam, reg32 lParam,
                  reg32 result);
// The game's 0x4cce40: WM_CLOSE.
reg32 onClose(App* app, CPU& cpu, reg32 window, reg32 hwnd, reg32 message, reg32 wParam, reg32 lParam, reg32 result);
// The game's 0x4cce60: the game's own message to close the window: the thread does it.
reg32 onQuitAsked(App* app, CPU& cpu, reg32 window, reg32 hwnd, reg32 message, reg32 wParam, reg32 lParam,
                  reg32 result);
// The game's 0x4cce80: a message said to be handled, and no more.
reg32 onDestroy(App* app, CPU& cpu, reg32 window, reg32 hwnd, reg32 message, reg32 wParam, reg32 lParam, reg32 result);
// The game's 0x4cce90: WM_SYSCOMMAND: closing, sizing and the screen saver are nobody's in a fullscreen window.
reg32 onSysCommand(App* app, CPU& cpu, reg32 window, reg32 hwnd, reg32 message, reg32 wParam, reg32 lParam,
                   reg32 result);
// The game's 0x4cced0: WM_SETTINGCHANGE: in fullscreen, they are turned off again.
reg32 onSettingChange(App* app, CPU& cpu, reg32 window, reg32 hwnd, reg32 message, reg32 wParam, reg32 lParam,
                      reg32 result);
// The game's 0x4ccf50: the window was moved or sized: its place kept, the resize told.
reg32 onMoved(App* app, CPU& cpu, reg32 window, reg32 hwnd, reg32 message, reg32 wParam, reg32 lParam, reg32 result);
// The game's 0x4ccfc0: WM_CHAR: the character on the key queue, as many times as it repeats.
reg32 onChar(App* app, CPU& cpu, reg32 window, reg32 hwnd, reg32 message, reg32 wParam, reg32 lParam, reg32 result);
// The game's 0x4cd020: WM_KEYDOWN: the key down in the table, and on the queue when it is one the game asks for by
// scan code or an arrow or function key.
reg32 onKeyDown(App* app, CPU& cpu, reg32 window, reg32 hwnd, reg32 message, reg32 wParam, reg32 lParam, reg32 result);
// The game's 0x4cd0e0: WM_KEYUP: the key up in the table; the two shifts go together.
reg32 onKeyUp(App* app, CPU& cpu, reg32 window, reg32 hwnd, reg32 message, reg32 wParam, reg32 lParam, reg32 result);
// The game's 0x4cd130: a mouse button pressed.
reg32 onMouseDown(App* app, CPU& cpu, reg32 window, reg32 hwnd, reg32 message, reg32 wParam, reg32 lParam,
                  reg32 result);
// The game's 0x4cd190: a mouse button let go, or the mouse moved.
reg32 onMouseUp(App* app, CPU& cpu, reg32 window, reg32 hwnd, reg32 message, reg32 wParam, reg32 lParam, reg32 result);
// The game's 0x4cd1f0: WM_PAINT: painted with nothing, under the graphics' lock.
reg32 onPaint(App* app, CPU& cpu, reg32 window, reg32 hwnd, reg32 message, reg32 wParam, reg32 lParam, reg32 result);
// The game's 0x4cd2a0: WM_ERASEBKGND of the fullscreen window: said to be done.
reg32 onEraseBackground(App* app, CPU& cpu, reg32 window, reg32 hwnd, reg32 message, reg32 wParam, reg32 lParam,
                        reg32 result);
// The game's 0x4cd2d0: the window's procedure: the table's handler, else the system's.
reg32 windowProc(App* app, CPU& cpu, reg32 hwnd, reg32 message, reg32 wParam, reg32 lParam);
// The game's 0x4cd350: the window created, fullscreen or framed; its handle.  With none it stays as it is; the
// arrows and such of the scan table's three keyboards are set up.
reg32 createWindow(App* app, CPU& cpu, reg32 window);
// The game's 0x4cd4e0: the window's thread: makes the window, tells who waits, pumps its messages until it is
// gone.
reg32 windowThread(App* app, CPU& cpu);
// The game's 0x4cd6c0: the game's window, opened or changed for a mode: the class and the handlers the first time,
// its thread each time; its handle.
reg32 openWindow(App* app, CPU& cpu, reg32 width, reg32 height, reg32 isFullscreen);
// The game's 0x4cdaa0: the window closed: the keys the system had put back, and waited for.
reg32 closeWindow(App* app, CPU& cpu);
// The game's 0x4cdb10: the window put at a place with a size of client area; what came kept.
reg32 placeWindow(App* app, CPU& cpu, reg32 left, reg32 top, reg32 width, reg32 height);
// The game's 0x4cdbd0: the window centred on the screen.
reg32 centreWindow(App* app, CPU& cpu, reg32 width, reg32 height);
// The game's 0x4cdc50: the keyboard hook off, in fullscreen.
reg32 hookOff(App* app, CPU& cpu);
// The game's 0x4cdc90: the keyboard hook on, with who is told.
reg32 hookOn(App* app, CPU& cpu, reg32 alt, reg32 callback);
// The game's 0x4fb470: the mouse: the place held inside the clip, the system's cursor put back when it was
// outside, the place, buttons and the click kept under the table's lock.
reg32 pointerMoved(App* app, CPU& cpu, reg32 x, reg32 y, reg32 buttons, reg32 click);
// The game's 0x4a38c0: whether the game's window is up already.
reg32 alreadyRunning(App* app, CPU& cpu);
// The game's 0x4a38f0: the file version of the game's exe, its text into the buffer given; 1 when it has one.
reg32 versionText(App* app, CPU& cpu, reg32 buffer, reg32 length);
// The game's 0x4b56d0: an import's answer, no more.
reg32 tickCount(App* app, CPU& cpu);
// The game's 0x4b5860: DirectX 5 and a sound or graphics library there, or the game says so in the player's
// language and leaves.
reg32 checkLibrary(App* app, CPU& cpu, reg32 library, reg32 language);
// The game's 0x4b5e50: a message of two tables, by the language, and the game leaves.
reg32 fatalMessage(App* app, CPU& cpu, reg32 language);
// The game's 0x4b5f10: enough memory and disk for the game, or the player is asked.
reg32 checkMemory(App* app, CPU& cpu, reg32 language);
// The game's 0x4b5ff0: the answer to the Direct3D device enumeration's question.
reg32 deviceAnswer(App* app, CPU& cpu, reg32 word0, reg32 word1, reg32 word2, reg32 question, reg32 word4, reg32 word5);
// The game's 0x4b6400: the 3D device's name and its memory from the registry; a name that says so when it has
// none.
reg32 thrashDevice(App* app, CPU& cpu, reg32 buffer);
// The game's 0x4b64c0: the 3D device's description from the registry.
reg32 thrashDescription(App* app, CPU& cpu, reg32 buffer);
// The game's 0x4b6520: the desktop's wallpaper setting from the registry.
reg32 desktopSetting(App* app, CPU& cpu);
// The game's 0x4b6590: what DirectX there is: the system's version, then ddraw.dll's interfaces asked for one by
// one: 0 none, 1, 3 or 5.
reg32 directXVersion(App* app, CPU& cpu);
// The game's 0x4ea665: entry `index` of a table, or 0 past its end.
reg32 tableEntry(App* app, CPU& cpu, reg32 table, reg32 index, reg32 out);
// The game's 0x4f2460: a shape's hot spots, the attachment of type 0x7c in its chain; 0 when none. A shape of a
// kind not known is said to be wrong.
reg32 shapeHotspots(App* app, CPU& cpu, reg32 shape);
// The game's 0x4f87a0: the graphics' screen lock let go and its driver slot back.
reg32 releaseDriverSlot(App* app, CPU& cpu);
// The game's 0x4f9120: whether the game has the focus and a window; else sub_4f9088 says.
reg32 hasFocus(App* app, CPU& cpu);
// The game's 0x4f9360: the video buffers given back.
reg32 videoClose(App* app, CPU& cpu);
// The game's 0x4fb060: a block of memory the race kept, let go.
reg32 freeKeptBlock(App* app, CPU& cpu);
// The game's 0x4fd490: a row filled, its length halved.
reg32 fillHalved(App* app, CPU& cpu, reg32 to, reg32 count);
// The game's 0x4fd4b0: a row filled, the count as it is.
reg32 fillSame(App* app, CPU& cpu, reg32 to, reg32 count);
// The game's 0x4fd4c0: a row filled, the count doubled.
reg32 fillDoubled(App* app, CPU& cpu, reg32 to, reg32 count);
// The game's 0x4fd520: a row filled, the count times four.
reg32 fillQuadrupled(App* app, CPU& cpu, reg32 to, reg32 count);
// The game's 0x4fd4e0: 16-bit pixels of the fill colour.
reg32 fillWords(App* app, CPU& cpu, reg32 upper, reg32 to, reg32 count);
// The game's 0x4fd500: 24-bit pixels of the fill colour.
reg32 fillTriples(App* app, CPU& cpu, reg32 to, reg32 count);
// The game's 0x503b00: an int shifted left into 64 bits at esi.
reg32 shiftTo64(App* app, CPU& cpu, reg32 value, reg32 by, reg32 out);
// The game's 0x503cd0: palette entries given to the window's surface, read back unless it is in fullscreen.
reg32 paletteToSurface(App* app, CPU& cpu, reg32 surface, reg32 first, reg32 count);
// The game's 0x503e50: palette entries set from bytes red green blue.
reg32 paletteSet(App* app, CPU& cpu, reg32 first, reg32 count, reg32 from);
// The game's 0x503ec0: palette entries as bytes red green blue.
reg32 paletteGet(App* app, CPU& cpu, reg32 first, reg32 count, reg32 to);
// The game's 0x50e800: the queue's first entry of a priority taken off it, 0 when it has none.
reg32 queueTakeByPriority(App* app, CPU& cpu, reg32 queue, reg32 wanted);
// The game's 0x517580: the graphics closed when they are open.
reg32 graphicsClose(App* app, CPU& cpu);
// The game's 0x5175b0: the system's request to close, from whichever thread.
reg32 graphicsAskClose(App* app, CPU& cpu);
// The game's 0x517920: the screen's palette, or a shape's, given to the driver.
reg32 graphicsPalette(App* app, CPU& cpu, reg32 surface);
// The game's 0x519140: the video's two buffers given back through the driver's table.
reg32 releaseVideoBuffers(App* app, CPU& cpu);
// The game's 0x51a170: a surface given back through the driver's table.
reg32 freeSurface(App* app, CPU& cpu, reg32 surface);
// The game's 0x5244f0: the system's tasks, the idle task of the graphics.
reg32 graphicsIdle(App* app, CPU& cpu);
// The game's 0x524500: the surface's interface asked for, or the object let go.
reg32 setDirectDraw(App* app, CPU& cpu, reg32 object);
// The game's 0x524550: the graphics' lock taken or let go, as the argument says.
reg32 graphicsLock(App* app, CPU& cpu, reg32 taking);
// The game's 0x5245a0: a message of the driver's: an error, on the monochrome screen, in the log, or to the
// debugger, by its level.
reg32 driverMessage(App* app, CPU& cpu, reg32 level, reg32 text);
// The game's 0x5245f0: the driver's event: the resize callback told on the second kind.
reg32 driverEvent(App* app, CPU& cpu, reg32 kind, reg32 detail);
// The game's 0x524620: the driver closed without its window, for an error.
reg32 closeDriverOnly(App* app, CPU& cpu);
// The game's 0x5247c0: the screen's size and depth, with the defaults; the graphics started when they are not.
reg32 setScreenSize(App* app, CPU& cpu, reg32 widthIn, reg32 heightIn, reg32 depthIn);
// The game's 0x525600: what a surface keeps on the side, given back.
reg32 freeSurfaceCache(App* app, CPU& cpu, reg32 surface);
// The game's 0x525630: the next byte of a text, its pointer moved.
reg32 nextByte(App* app, CPU& cpu, reg32 place);
// The game's 0x525650: a character through the table of the printable ones.
reg32 mapCharacter(App* app, CPU& cpu, reg32 character);
// The game's 0x525670: the next character of a text: two bytes with the top bit set, else one through the table.
reg32 nextCharacter(App* app, CPU& cpu, reg32 place);
// The game's 0x52e420: the text driver started with the screen's saved two values.
reg32 textStart(App* app, CPU& cpu);

}  // namespace wina
}  // namespace frontend

namespace guest
{

// handle eax, streamOut edx, handleOut ebx.
template <>
struct ByAddress<frontend::wina::streamCheck>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32 streamOut,
                           x86::reg32 handleOut)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ handle, streamOut, handleOut }));
    }
};

// from eax, to edx, place ebx.
template <>
struct ByAddress<frontend::wina::ringBetween>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 from, x86::reg32 to, x86::reg32 place)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ from, to, place }));
    }
};

// s eax, amount edx.
template <>
struct ByAddress<frontend::wina::streamConsume>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 s, x86::reg32 amount)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ s, amount }));
    }
};

// s eax.
template <>
struct ByAddress<frontend::wina::streamTakeFree>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 s)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ s }));
    }
};

// s eax, record edx.
template <>
struct ByAddress<frontend::wina::streamQueueFile>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 s, x86::reg32 record)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ s, record }));
    }
};

// s eax, id edx.
template <>
struct ByAddress<frontend::wina::streamFindFile>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 s, x86::reg32 id)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ s, id }));
    }
};

// s eax, record edx.
template <>
struct ByAddress<frontend::wina::streamUnlinkFile>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 s, x86::reg32 record)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ s, record }));
    }
};

// s eax, chunk edx.
template <>
struct ByAddress<frontend::wina::streamClassify>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 s, x86::reg32 chunk)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ s, chunk }));
    }
};

// s eax.
template <>
struct ByAddress<frontend::wina::streamParse>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 s)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ s }));
    }
};

// s ebx.
template <>
struct ByAddress<frontend::wina::streamFileOpened>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 s)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, cpu.edx, s }));
    }
};

// s ebx.
template <>
struct ByAddress<frontend::wina::streamFileSwitched>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 s)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, cpu.edx, s }));
    }
};

// s ebx.
template <>
struct ByAddress<frontend::wina::streamFilled>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 s)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, cpu.edx, s }));
    }
};

// s eax, priority edx.
template <>
struct ByAddress<frontend::wina::streamNextFile>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 s, x86::reg32 priority)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ s, priority }));
    }
};

// s eax, priority edx.
template <>
struct ByAddress<frontend::wina::streamReadMore>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 s, x86::reg32 priority)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ s, priority }));
    }
};

// fileCount eax, ruleCount edx, channelCount ebx.
template <>
struct ByAddress<frontend::wina::streamHeaderSize>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 fileCount, x86::reg32 ruleCount,
                           x86::reg32 channelCount)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ fileCount, ruleCount, channelCount }));
    }
};

// files eax, rules edx, channels ebx, s ecx, memory stack 0.
template <>
struct ByAddress<frontend::wina::streamCreate>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 files, x86::reg32 rules,
                           x86::reg32 channels, x86::reg32 s, x86::reg32 memory)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ files, rules, channels, s }, { memory }));
    }
};

// handle eax, number edx, mask ebx, value ecx, target stack 0.
template <>
struct ByAddress<frontend::wina::streamSetRule>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32 number,
                           x86::reg32 mask, x86::reg32 value, x86::reg32 target)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ handle, number, mask, value }, { target }));
    }
};

// handle eax.
template <>
struct ByAddress<frontend::wina::streamClose>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ handle }));
    }
};

// handle eax, low edx.
template <>
struct ByAddress<frontend::wina::streamSetLowWater>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32 low)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ handle, low }));
    }
};

// handle eax, below edx.
template <>
struct ByAddress<frontend::wina::streamSetBelow>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32 below)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ handle, below }));
    }
};

// handle eax, number edx.
template <>
struct ByAddress<frontend::wina::streamChannel>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32 number)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ handle, number }));
    }
};

// handle eax, name edx, length ebx, endId ecx.
template <>
struct ByAddress<frontend::wina::streamOpenFile>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32 name,
                           x86::reg32 length, x86::reg32 endId)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ handle, name, length, endId }));
    }
};

// handle eax, data edx, length ebx, endId ecx.
template <>
struct ByAddress<frontend::wina::streamOpenMemory>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32 data,
                           x86::reg32 length, x86::reg32 endId)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ handle, data, length, endId }));
    }
};

// handle eax, id edx.
template <>
struct ByAddress<frontend::wina::streamCloseFile>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32 id)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ handle, id }));
    }
};

// handle eax.
template <>
struct ByAddress<frontend::wina::streamFlush>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ handle }));
    }
};

// handle eax.
template <>
struct ByAddress<frontend::wina::streamNextChunk>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ handle }));
    }
};

// handle eax, chunk edx.
template <>
struct ByAddress<frontend::wina::streamFreeChunk>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32 chunk)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ handle, chunk }));
    }
};

// handle eax.
template <>
struct ByAddress<frontend::wina::channelBytes>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ handle }));
    }
};

// handle eax.
template <>
struct ByAddress<frontend::wina::channelState>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ handle }));
    }
};

// handle eax.
template <>
struct ByAddress<frontend::wina::channelDone>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ handle }));
    }
};

// handle eax.
template <>
struct ByAddress<frontend::wina::channelRingSize>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ handle }));
    }
};

// reader eax.
template <>
struct ByAddress<frontend::wina::movieNextFrame>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 reader)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ reader }));
    }
};

// kind eax, priorityIn edx, target ebx, slot ecx.
template <>
struct ByAddress<frontend::wina::fileOpAllocate>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 kind, x86::reg32 priorityIn,
                           x86::reg32 target, x86::reg32 slot)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ kind, priorityIn, target, slot }));
    }
};

// op eax.
template <>
struct ByAddress<frontend::wina::fileOpQueue>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 op)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ op }));
    }
};

// op eax, nameIn edx.
template <>
struct ByAddress<frontend::wina::fileOpSetName>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 op, x86::reg32 nameIn)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ op, nameIn }));
    }
};

// id eax, callback edx.
template <>
struct ByAddress<frontend::wina::fileOpCallback>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 id, x86::reg32 callback)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ id, callback }));
    }
};

// id eax, priority edx.
template <>
struct ByAddress<frontend::wina::fileOpPriority>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 id, x86::reg32 priority)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ id, priority }));
    }
};

// name eax, mode edx, priority ebx, target ecx.
template <>
struct ByAddress<frontend::wina::fileOpOpen>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name, x86::reg32 mode,
                           x86::reg32 priority, x86::reg32 target)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ name, mode, priority, target }));
    }
};

// handle eax, priority edx, target ebx.
template <>
struct ByAddress<frontend::wina::fileOpClose>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32 priority,
                           x86::reg32 target)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ handle, priority, target }));
    }
};

// handle eax, offset edx, buffer ebx, size ecx, priority stack 0, target stack 1.
template <>
struct ByAddress<frontend::wina::fileOpRead>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32 offset,
                           x86::reg32 buffer, x86::reg32 size, x86::reg32 priority, x86::reg32 target)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ handle, offset, buffer, size }, { priority, target }));
    }
};

// name eax.
template <>
struct ByAddress<frontend::wina::fileSlotOfName>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 name)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ name }));
    }
};

// handle eax, priority edx.
template <>
struct ByAddress<frontend::wina::fileCloseWait>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32 priority)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ handle, priority }));
    }
};

// handle eax, offset edx.
template <>
struct ByAddress<frontend::wina::fileSizeAndSeek>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle, x86::reg32 offset)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ handle, offset }));
    }
};

// handle eax.
template <>
struct ByAddress<frontend::wina::fileLockOf>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handle)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ handle }));
    }
};

// leftRow eax, rightRow edx.
template <>
struct ByAddress<frontend::wina::compareHandlers>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 leftRow, x86::reg32 rightRow)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ leftRow, rightRow }));
    }
};

// message stack 0.
template <>
struct ByAddress<frontend::wina::handlerOf>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 message)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { message }));
    }
};

// message stack 0, handler stack 1.
template <>
struct ByAddress<frontend::wina::handlerSet>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 message, x86::reg32 handler)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { message, handler }));
    }
};

// message eax, lParam edx, wParam ebx, send ecx.
template <>
struct ByAddress<frontend::wina::postMessage>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 message, x86::reg32 lParam,
                           x86::reg32 wParam, x86::reg32 send)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ message, lParam, wParam, send }));
    }
};

// callback eax.
template <>
struct ByAddress<frontend::wina::setFocusCallback>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 callback)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ callback }));
    }
};

template <>
struct ByAddress<frontend::wina::noWindowTask>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// window eax, x edx, y ebx.
template <>
struct ByAddress<frontend::wina::movePointer>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 window, x86::reg32 x, x86::reg32 y)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ window, x, y }));
    }
};

// key eax, scan edx, flags ebx.
template <>
struct ByAddress<frontend::wina::pushKey>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 key, x86::reg32 scan, x86::reg32 flags)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ key, scan, flags }));
    }
};

// message eax, virtualKey edx.
template <>
struct ByAddress<frontend::wina::keyOfMessage>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 message, x86::reg32 virtualKey)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ message, virtualKey }));
    }
};

// code stack 0, wParam stack 1, lParam stack 2.
template <>
struct ByAddress<frontend::wina::keyboardHook>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 code, x86::reg32 wParam,
                           x86::reg32 lParam)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { code, wParam, lParam }));
    }
};

// window stack 0, hwnd stack 1, message stack 2, wParam stack 3, lParam stack 4, result stack 5.
template <>
struct ByAddress<frontend::wina::onSetFocus>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 window, x86::reg32 hwnd,
                           x86::reg32 message, x86::reg32 wParam, x86::reg32 lParam, x86::reg32 result)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { window, hwnd, message, wParam, lParam, result }));
    }
};

// window stack 0, hwnd stack 1, message stack 2, wParam stack 3, lParam stack 4, result stack 5.
template <>
struct ByAddress<frontend::wina::onKillFocus>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 window, x86::reg32 hwnd,
                           x86::reg32 message, x86::reg32 wParam, x86::reg32 lParam, x86::reg32 result)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { window, hwnd, message, wParam, lParam, result }));
    }
};

// window stack 0, hwnd stack 1, message stack 2, wParam stack 3, lParam stack 4, result stack 5.
template <>
struct ByAddress<frontend::wina::onClose>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 window, x86::reg32 hwnd,
                           x86::reg32 message, x86::reg32 wParam, x86::reg32 lParam, x86::reg32 result)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { window, hwnd, message, wParam, lParam, result }));
    }
};

// window stack 0, hwnd stack 1, message stack 2, wParam stack 3, lParam stack 4, result stack 5.
template <>
struct ByAddress<frontend::wina::onQuitAsked>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 window, x86::reg32 hwnd,
                           x86::reg32 message, x86::reg32 wParam, x86::reg32 lParam, x86::reg32 result)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { window, hwnd, message, wParam, lParam, result }));
    }
};

// window stack 0, hwnd stack 1, message stack 2, wParam stack 3, lParam stack 4, result stack 5.
template <>
struct ByAddress<frontend::wina::onDestroy>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 window, x86::reg32 hwnd,
                           x86::reg32 message, x86::reg32 wParam, x86::reg32 lParam, x86::reg32 result)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { window, hwnd, message, wParam, lParam, result }));
    }
};

// window stack 0, hwnd stack 1, message stack 2, wParam stack 3, lParam stack 4, result stack 5.
template <>
struct ByAddress<frontend::wina::onSysCommand>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 window, x86::reg32 hwnd,
                           x86::reg32 message, x86::reg32 wParam, x86::reg32 lParam, x86::reg32 result)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { window, hwnd, message, wParam, lParam, result }));
    }
};

// window stack 0, hwnd stack 1, message stack 2, wParam stack 3, lParam stack 4, result stack 5.
template <>
struct ByAddress<frontend::wina::onSettingChange>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 window, x86::reg32 hwnd,
                           x86::reg32 message, x86::reg32 wParam, x86::reg32 lParam, x86::reg32 result)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { window, hwnd, message, wParam, lParam, result }));
    }
};

// window stack 0, hwnd stack 1, message stack 2, wParam stack 3, lParam stack 4, result stack 5.
template <>
struct ByAddress<frontend::wina::onMoved>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 window, x86::reg32 hwnd,
                           x86::reg32 message, x86::reg32 wParam, x86::reg32 lParam, x86::reg32 result)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { window, hwnd, message, wParam, lParam, result }));
    }
};

// window stack 0, hwnd stack 1, message stack 2, wParam stack 3, lParam stack 4, result stack 5.
template <>
struct ByAddress<frontend::wina::onChar>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 window, x86::reg32 hwnd,
                           x86::reg32 message, x86::reg32 wParam, x86::reg32 lParam, x86::reg32 result)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { window, hwnd, message, wParam, lParam, result }));
    }
};

// window stack 0, hwnd stack 1, message stack 2, wParam stack 3, lParam stack 4, result stack 5.
template <>
struct ByAddress<frontend::wina::onKeyDown>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 window, x86::reg32 hwnd,
                           x86::reg32 message, x86::reg32 wParam, x86::reg32 lParam, x86::reg32 result)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { window, hwnd, message, wParam, lParam, result }));
    }
};

// window stack 0, hwnd stack 1, message stack 2, wParam stack 3, lParam stack 4, result stack 5.
template <>
struct ByAddress<frontend::wina::onKeyUp>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 window, x86::reg32 hwnd,
                           x86::reg32 message, x86::reg32 wParam, x86::reg32 lParam, x86::reg32 result)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { window, hwnd, message, wParam, lParam, result }));
    }
};

// window stack 0, hwnd stack 1, message stack 2, wParam stack 3, lParam stack 4, result stack 5.
template <>
struct ByAddress<frontend::wina::onMouseDown>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 window, x86::reg32 hwnd,
                           x86::reg32 message, x86::reg32 wParam, x86::reg32 lParam, x86::reg32 result)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { window, hwnd, message, wParam, lParam, result }));
    }
};

// window stack 0, hwnd stack 1, message stack 2, wParam stack 3, lParam stack 4, result stack 5.
template <>
struct ByAddress<frontend::wina::onMouseUp>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 window, x86::reg32 hwnd,
                           x86::reg32 message, x86::reg32 wParam, x86::reg32 lParam, x86::reg32 result)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { window, hwnd, message, wParam, lParam, result }));
    }
};

// window stack 0, hwnd stack 1, message stack 2, wParam stack 3, lParam stack 4, result stack 5.
template <>
struct ByAddress<frontend::wina::onPaint>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 window, x86::reg32 hwnd,
                           x86::reg32 message, x86::reg32 wParam, x86::reg32 lParam, x86::reg32 result)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { window, hwnd, message, wParam, lParam, result }));
    }
};

// window stack 0, hwnd stack 1, message stack 2, wParam stack 3, lParam stack 4, result stack 5.
template <>
struct ByAddress<frontend::wina::onEraseBackground>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 window, x86::reg32 hwnd,
                           x86::reg32 message, x86::reg32 wParam, x86::reg32 lParam, x86::reg32 result)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { window, hwnd, message, wParam, lParam, result }));
    }
};

// hwnd stack 0, message stack 1, wParam stack 2, lParam stack 3.
template <>
struct ByAddress<frontend::wina::windowProc>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 hwnd, x86::reg32 message,
                           x86::reg32 wParam, x86::reg32 lParam)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { hwnd, message, wParam, lParam }));
    }
};

// window eax.
template <>
struct ByAddress<frontend::wina::createWindow>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 window)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ window }));
    }
};

template <>
struct ByAddress<frontend::wina::windowThread>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// width eax, height edx, isFullscreen ebx.
template <>
struct ByAddress<frontend::wina::openWindow>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 width, x86::reg32 height,
                           x86::reg32 isFullscreen)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ width, height, isFullscreen }));
    }
};

template <>
struct ByAddress<frontend::wina::closeWindow>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// left eax, top edx, width ebx, height ecx.
template <>
struct ByAddress<frontend::wina::placeWindow>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 left, x86::reg32 top, x86::reg32 width,
                           x86::reg32 height)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ left, top, width, height }));
    }
};

// width eax, height edx.
template <>
struct ByAddress<frontend::wina::centreWindow>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 width, x86::reg32 height)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ width, height }));
    }
};

template <>
struct ByAddress<frontend::wina::hookOff>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// alt eax, callback edx.
template <>
struct ByAddress<frontend::wina::hookOn>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 alt, x86::reg32 callback)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ alt, callback }));
    }
};

// x eax, y edx, buttons ebx, click ecx.
template <>
struct ByAddress<frontend::wina::pointerMoved>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 x, x86::reg32 y, x86::reg32 buttons,
                           x86::reg32 click)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ x, y, buttons, click }));
    }
};

template <>
struct ByAddress<frontend::wina::alreadyRunning>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// buffer eax, length edx.
template <>
struct ByAddress<frontend::wina::versionText>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buffer, x86::reg32 length)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ buffer, length }));
    }
};

template <>
struct ByAddress<frontend::wina::tickCount>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// library eax, language edx.
template <>
struct ByAddress<frontend::wina::checkLibrary>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 library, x86::reg32 language)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ library, language }));
    }
};

// language eax.
template <>
struct ByAddress<frontend::wina::fatalMessage>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 language)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ language }));
    }
};

// language eax.
template <>
struct ByAddress<frontend::wina::checkMemory>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 language)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ language }));
    }
};

// word0 stack 0, word1 stack 1, word2 stack 2, question stack 3, word4 stack 4, word5 stack 5.
template <>
struct ByAddress<frontend::wina::deviceAnswer>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 word0, x86::reg32 word1,
                           x86::reg32 word2, x86::reg32 question, x86::reg32 word4, x86::reg32 word5)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { word0, word1, word2, question, word4, word5 }));
    }
};

// buffer eax.
template <>
struct ByAddress<frontend::wina::thrashDevice>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buffer)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ buffer }));
    }
};

// buffer eax.
template <>
struct ByAddress<frontend::wina::thrashDescription>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buffer)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ buffer }));
    }
};

template <>
struct ByAddress<frontend::wina::desktopSetting>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wina::directXVersion>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// table eax, index edx, out ebx.
template <>
struct ByAddress<frontend::wina::tableEntry>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 table, x86::reg32 index,
                           x86::reg32 out)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ table, index, out }));
    }
};

// shape eax.
template <>
struct ByAddress<frontend::wina::shapeHotspots>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 shape)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ shape }));
    }
};

template <>
struct ByAddress<frontend::wina::releaseDriverSlot>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wina::hasFocus>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wina::videoClose>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wina::freeKeptBlock>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// to edx, count ebx.
template <>
struct ByAddress<frontend::wina::fillHalved>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 to, x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, to, count }));
    }
};

// to edx, count ebx.
template <>
struct ByAddress<frontend::wina::fillSame>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 to, x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, to, count }));
    }
};

// to edx, count ebx.
template <>
struct ByAddress<frontend::wina::fillDoubled>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 to, x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, to, count }));
    }
};

// to edx, count ebx.
template <>
struct ByAddress<frontend::wina::fillQuadrupled>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 to, x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, to, count }));
    }
};

// upper eax, to edx, count ebx.
template <>
struct ByAddress<frontend::wina::fillWords>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 upper, x86::reg32 to, x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ upper, to, count }));
    }
};

// to edx, count ebx.
template <>
struct ByAddress<frontend::wina::fillTriples>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 to, x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ cpu.eax, to, count }));
    }
};

// value eax, by edx, out esi.
template <>
struct ByAddress<frontend::wina::shiftTo64>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 value, x86::reg32 by, x86::reg32 out)
    {
        cpu.esi = out;
        return fe::callByAddress(app, cpu, Address, fe::args({ value, by }));
    }
};

// surface eax, first edx, count ebx.
template <>
struct ByAddress<frontend::wina::paletteToSurface>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 surface, x86::reg32 first,
                           x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ surface, first, count }));
    }
};

// first eax, count edx, from ebx.
template <>
struct ByAddress<frontend::wina::paletteSet>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 first, x86::reg32 count,
                           x86::reg32 from)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ first, count, from }));
    }
};

// first eax, count edx, to ebx.
template <>
struct ByAddress<frontend::wina::paletteGet>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 first, x86::reg32 count, x86::reg32 to)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ first, count, to }));
    }
};

// queue eax, wanted edx.
template <>
struct ByAddress<frontend::wina::queueTakeByPriority>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 queue, x86::reg32 wanted)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ queue, wanted }));
    }
};

template <>
struct ByAddress<frontend::wina::graphicsClose>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::wina::graphicsAskClose>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// surface eax.
template <>
struct ByAddress<frontend::wina::graphicsPalette>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 surface)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ surface }));
    }
};

template <>
struct ByAddress<frontend::wina::releaseVideoBuffers>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// surface eax.
template <>
struct ByAddress<frontend::wina::freeSurface>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 surface)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ surface }));
    }
};

template <>
struct ByAddress<frontend::wina::graphicsIdle>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// object stack 0.
template <>
struct ByAddress<frontend::wina::setDirectDraw>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 object)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { object }));
    }
};

// taking stack 0.
template <>
struct ByAddress<frontend::wina::graphicsLock>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 taking)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { taking }));
    }
};

// level stack 0, text stack 1.
template <>
struct ByAddress<frontend::wina::driverMessage>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 level, x86::reg32 text)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { level, text }));
    }
};

// kind stack 0, detail stack 1.
template <>
struct ByAddress<frontend::wina::driverEvent>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 kind, x86::reg32 detail)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}, { kind, detail }));
    }
};

template <>
struct ByAddress<frontend::wina::closeDriverOnly>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// widthIn eax, heightIn edx, depthIn ebx.
template <>
struct ByAddress<frontend::wina::setScreenSize>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 widthIn, x86::reg32 heightIn,
                           x86::reg32 depthIn)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ widthIn, heightIn, depthIn }));
    }
};

// surface eax.
template <>
struct ByAddress<frontend::wina::freeSurfaceCache>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 surface)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ surface }));
    }
};

// place eax.
template <>
struct ByAddress<frontend::wina::nextByte>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 place)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ place }));
    }
};

// character eax.
template <>
struct ByAddress<frontend::wina::mapCharacter>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 character)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ character }));
    }
};

// place eax.
template <>
struct ByAddress<frontend::wina::nextCharacter>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 place)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ place }));
    }
};

template <>
struct ByAddress<frontend::wina::textStart>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
