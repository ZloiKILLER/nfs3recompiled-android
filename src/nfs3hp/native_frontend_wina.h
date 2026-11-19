#ifndef NFS3HP_NATIVE_FRONTEND_WINA_H
#define NFS3HP_NATIVE_FRONTEND_WINA_H

#include "native_frontend.h"
#include "native_guest.h"
#include "os_window.h"

/* native_frontend_wina.cpp's typed functions (the buffered stream under the movie player, EA's file
 * library's operations, the game's window and its messages, the startup checks, what is left of the
 * graphics library, the palette and the runtime), which other files call as C++ (guest::direct),
 * and how the game's code calls each by address.  Each calls
 * the game's other functions by address on `ts` and throws fe::Terminated when the game stopped in
 * one, as a call by address does; its result is what the game's function leaves in eax.  A register past
 * the parameters a function takes is handed on as the caller has it. */
namespace nfs3hp
{
namespace frontend
{
namespace wina
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x4cf1c0: a handle to its stream, both handed out; nonzero when it is none.
u32 streamCheck(App* app, ThreadState& ts, u32 handle, u32 streamOut, u32 handleOut);
// The game's 0x4cf1f0: whether a place of the ring is between two others, the way round.
u32 ringBetween(App* app, ThreadState& ts, u32 from, u32 to, u32 place);
// The game's 0x4cf220: bytes taken off what is queued; below the low water, more is asked for sooner: the read in
// hand gets the busy priority.
u32 streamConsume(App* app, ThreadState& ts, u32 s, u32 amount);
// The game's 0x4cf280: a file record off the free list, its id's serial moved on; 0 when none.
u32 streamTakeFree(App* app, ThreadState& ts, u32 s);
// The game's 0x4cf2f0: a file record on the end of the queue.
u32 streamQueueFile(App* app, ThreadState& ts, u32 s, u32 record);
// The game's 0x4cf340: the file record of an id, 0 when it has gone.
u32 streamFindFile(App* app, ThreadState& ts, u32 s, u32 id);
// The game's 0x4cf380: a file record off the queue, onto the free list.
u32 streamUnlinkFile(App* app, ThreadState& ts, u32 s, u32 record);
// The game's 0x4cf3f0: the channel a chunk's id goes to, by the rules (mask, value, channel); -2 none.
u32 streamClassify(App* app, ThreadState& ts, u32 s, u32 chunk);
// The game's 0x4cf440: the whole chunks read so far given to their channels; 1 when the current file's last chunk
// has come.
u32 streamParse(App* app, ThreadState& ts, u32 s);
// The game's 0x4cf570: the file's open has ended: its handle kept, reading begins.
u32 streamFileOpened(App* app, ThreadState& ts, u32 s);
// The game's 0x4cf5a0: the last file's close has ended: the next one opened.
u32 streamFileSwitched(App* app, ThreadState& ts, u32 s);
// The game's 0x4cf5e0: a read has ended, or a memory block was copied: the chunks parsed, and on to more of this
// file or the next.
u32 streamFilled(App* app, ThreadState& ts, u32 s);
// The game's 0x4cf6b0: on to the next file of the queue: from memory at once, from a file by an open, after a
// close when another one is open.
u32 streamNextFile(App* app, ThreadState& ts, u32 s, u32 priority);
// The game's 0x4cf810: the next read into the ring: records of files read through let go, the ring wrapped when
// the end is too near, and 0x2000 bytes asked for or copied.
u32 streamReadMore(App* app, ThreadState& ts, u32 s, u32 priority);
// The game's 0x4cfa40: the bytes a stream's record and tables take.
u32 streamHeaderSize(App* app, ThreadState& ts, u32 fileCount, u32 ruleCount, u32 channelCount);
// The game's 0x4cfa80: a stream made in the memory given; its first channel, or 0 when it does not fit or asks too
// much.
u32 streamCreate(App* app, ThreadState& ts, u32 files, u32 rules, u32 channels, u32 s, u32 memory);
// The game's 0x4cfca0: a rule of a stream that is not reading: mask, value, channel.
u32 streamSetRule(App* app, ThreadState& ts, u32 handle, u32 number, u32 mask, u32 value, u32 target);
// The game's 0x4cfd30: a stream closed: flushed, its read waited for, its lock and file gone.
u32 streamClose(App* app, ThreadState& ts, u32 handle);
// The game's 0x4cfdd0: the low water mark: crossing it tells the read in hand.
u32 streamSetLowWater(App* app, ThreadState& ts, u32 handle, u32 low);
// The game's 0x4cfe30: whether the stream is below its low water; when it is, the read gets the busy priority.
u32 streamSetBelow(App* app, ThreadState& ts, u32 handle, u32 below);
// The game's 0x4cfe70: a channel's handle, by its number from 1; 0 when there is none.
u32 streamChannel(App* app, ThreadState& ts, u32 handle, u32 number);
// The game's 0x4cfef0: a file put on the queue; reading starts if it had stopped.  Its id, 0 when there is no
// room.
u32 streamOpenFile(App* app, ThreadState& ts, u32 handle, u32 name, u32 length, u32 endId);
// The game's 0x4cffb0: a block of memory put on the queue; with no length given, it runs to the chunk with the end
// id.  Its id, 0 when there is no room.
u32 streamOpenMemory(App* app, ThreadState& ts, u32 handle, u32 data, u32 length, u32 endId);
// The game's 0x4d0060: a file of the queue closed: what it has in the ring taken back from every channel, by
// freeing its chunks or marking them.
u32 streamCloseFile(App* app, ThreadState& ts, u32 handle, u32 id);
// The game's 0x4d0220: everything off a stream: its files closed, the channels emptied, the ring marked free.
u32 streamFlush(App* app, ThreadState& ts, u32 handle);
// The game's 0x4d0300: a channel's next chunk, its reader moved on past it; 0 when it has none.
u32 streamNextChunk(App* app, ThreadState& ts, u32 handle);
// The game's 0x4d03c0: a chunk read and done with: if reading waited for room, on it goes.
u32 streamFreeChunk(App* app, ThreadState& ts, u32 handle, u32 chunk);
// The game's 0x4d0460: the bytes a channel has waiting.
u32 channelBytes(App* app, ThreadState& ts, u32 handle);
// The game's 0x4d0490: the state of a channel's stream.
u32 channelState(App* app, ThreadState& ts, u32 handle);
// The game's 0x4d04c0: whether a channel has come to its end: nothing waiting, nothing reading.
u32 channelDone(App* app, ThreadState& ts, u32 handle);
// The game's 0x4d0500: how big a channel's stream's ring is.
u32 channelRingSize(App* app, ThreadState& ts, u32 handle);
// The game's 0x495b60: the movie's next frame chunk from its channel; what is not one is thrown away.  0 at the
// channel's end.  It lets other threads run at every try.
u32 movieNextFrame(App* app, ThreadState& ts, u32 reader);
// The game's 0x4fb990: a file operation off the free list, set up; its slot's serial in its id.
u32 fileOpAllocate(App* app, ThreadState& ts, u32 kind, u32 priorityIn, u32 target, u32 slot);
// The game's 0x4fbb60: an operation on its slot's queue, the slot's thread told; its id.
u32 fileOpQueue(App* app, ThreadState& ts, u32 op);
// The game's 0x4fbbe0: an operation's file name, a copy of it in memory of its own.
u32 fileOpSetName(App* app, ThreadState& ts, u32 op, u32 nameIn);
// The game's 0x4fc8e0: what is to be called when an operation ends; at once when it has.
u32 fileOpCallback(App* app, ThreadState& ts, u32 id, u32 callback);
// The game's 0x4fc9b0: an operation still queued given another priority.
u32 fileOpPriority(App* app, ThreadState& ts, u32 id, u32 priority);
// The game's 0x4fca60: an operation to open a file.
u32 fileOpOpen(App* app, ThreadState& ts, u32 name, u32 mode, u32 priority, u32 target);
// The game's 0x4fca90: an operation to close a file.
u32 fileOpClose(App* app, ThreadState& ts, u32 handle, u32 priority, u32 target);
// The game's 0x4fcab0: an operation to read a file.
u32 fileOpRead(App* app, ThreadState& ts, u32 handle, u32 offset, u32 buffer, u32 size, u32 priority,
                 u32 target);
// The game's 0x4fce50: the slot of a file's name: 0 when it has a '|', else by its drive.
u32 fileSlotOfName(App* app, ThreadState& ts, u32 name);
// The game's 0x4fd060: a file closed and waited for.
u32 fileCloseWait(App* app, ThreadState& ts, u32 handle, u32 priority);
// The game's 0x4e4230: a file's size, 0 when the handle is not of an open one or it keeps no system handle; the
// file pointer put where asked.  Under the table's own lock.
u32 fileSizeAndSeek(App* app, ThreadState& ts, u32 handle, u32 offset);
// The game's 0x4e4320: which lock of the table an open file's handle has; 0 when none.
u32 fileLockOf(App* app, ThreadState& ts, u32 handle);
// The game's 0x4cc650: two handler rows, by their message: for qsort and bsearch.
u32 compareHandlers(App* app, ThreadState& ts, u32 leftRow, u32 rightRow);
// The game's 0x4cc660: a message's handler, 0 when it has none.
u32 handlerOf(App* app, ThreadState& ts, u32 message);
// The game's 0x4cc6d0: a message's handler set, changed, or taken away with 0; the rows stay sorted.  1 when done,
// 0 when the table is full or it had none to take away.
u32 handlerSet(App* app, ThreadState& ts, u32 message, u32 handler);
// The game's 0x4cc7c0: a message to the game's window, sent or posted; 1 when it went.
u32 postMessage(App* app, ThreadState& ts, u32 message, u32 lParam, u32 wParam, u32 send);
// The game's 0x4cc860: who is told when the window gets or loses the focus.
u32 setFocusCallback(App* app, ThreadState& ts, u32 callback);
// The game's 0x4cc870: the system task that leaves the game when there is no window.
u32 noWindowTask(App* app, ThreadState& ts);
// The game's 0x4cc890: the system's cursor put at a place of the window.
u32 movePointer(App* app, ThreadState& ts, u32 window, u32 x, u32 y);
// The game's 0x4cc8e0: a key on the queue, the oldest dropped when it is full.
u32 pushKey(App* app, ThreadState& ts, u32 key, u32 scan, u32 flags);
// The game's 0x4ccaa0: the key code of the arrows and the function keys, from a key-down message's virtual key; 0
// for anything else.  The game's code's jumps are one table.
u32 keyOfMessage(App* app, ThreadState& ts, u32 message, u32 virtualKey);
// The game's 0x4ccc70: the low-level keyboard hook: Alt+Tab, Alt+Esc, Ctrl+Esc and the Windows keys told to the
// game instead of the system.
u32 keyboardHook(App* app, ThreadState& ts, u32 code, u32 wParam, u32 lParam);
// The game's 0x4ccd60: the window has the focus: hook and callback.
u32 onSetFocus(App* app, ThreadState& ts, u32 window, u32 hwnd, u32 message, u32 wParam, u32 lParam, u32 result);
// The game's 0x4ccdd0: the window has lost the focus: the hook off, the keys let go.
u32 onKillFocus(App* app, ThreadState& ts, u32 window, u32 hwnd, u32 message, u32 wParam, u32 lParam,
                  u32 result);
// The game's 0x4cce40: WM_CLOSE.
u32 onClose(App* app, ThreadState& ts, u32 window, u32 hwnd, u32 message, u32 wParam, u32 lParam, u32 result);
// The game's 0x4cce60: the game's own message to close the window: the thread does it.
u32 onQuitAsked(App* app, ThreadState& ts, u32 window, u32 hwnd, u32 message, u32 wParam, u32 lParam,
                  u32 result);
// The game's 0x4cce80: a message said to be handled, and no more.
u32 onDestroy(App* app, ThreadState& ts, u32 window, u32 hwnd, u32 message, u32 wParam, u32 lParam, u32 result);
// The game's 0x4cce90: WM_SYSCOMMAND: closing, sizing and the screen saver are nobody's in a fullscreen window.
u32 onSysCommand(App* app, ThreadState& ts, u32 window, u32 hwnd, u32 message, u32 wParam, u32 lParam,
                   u32 result);
// The game's 0x4cced0: WM_SETTINGCHANGE: in fullscreen, they are turned off again.
u32 onSettingChange(App* app, ThreadState& ts, u32 window, u32 hwnd, u32 message, u32 wParam, u32 lParam,
                      u32 result);
// The game's 0x4ccf50: the window was moved or sized: its place kept, the resize told.
u32 onMoved(App* app, ThreadState& ts, u32 window, u32 hwnd, u32 message, u32 wParam, u32 lParam, u32 result);
// The game's 0x4ccfc0: WM_CHAR: the character on the key queue, as many times as it repeats.
u32 onChar(App* app, ThreadState& ts, u32 window, u32 hwnd, u32 message, u32 wParam, u32 lParam, u32 result);
// The game's 0x4cd020: WM_KEYDOWN: the key down in the table, and on the queue when it is one the game asks for by
// scan code or an arrow or function key.
u32 onKeyDown(App* app, ThreadState& ts, u32 window, u32 hwnd, u32 message, u32 wParam, u32 lParam, u32 result);
// The game's 0x4cd0e0: WM_KEYUP: the key up in the table; the two shifts go together.
u32 onKeyUp(App* app, ThreadState& ts, u32 window, u32 hwnd, u32 message, u32 wParam, u32 lParam, u32 result);
// The game's 0x4cd130: a mouse button pressed.
u32 onMouseDown(App* app, ThreadState& ts, u32 window, u32 hwnd, u32 message, u32 wParam, u32 lParam,
                  u32 result);
// The game's 0x4cd190: a mouse button let go, or the mouse moved.
u32 onMouseUp(App* app, ThreadState& ts, u32 window, u32 hwnd, u32 message, u32 wParam, u32 lParam, u32 result);
// The game's 0x4cd1f0: WM_PAINT: painted with nothing, under the graphics' lock.
u32 onPaint(App* app, ThreadState& ts, u32 window, u32 hwnd, u32 message, u32 wParam, u32 lParam, u32 result);
// The game's 0x4cd2a0: WM_ERASEBKGND of the fullscreen window: said to be done.
u32 onEraseBackground(App* app, ThreadState& ts, u32 window, u32 hwnd, u32 message, u32 wParam, u32 lParam,
                        u32 result);
// The game's 0x4cd2d0: the window's procedure: the table's handler, else the port's.
u32 windowProc(App* app, ThreadState& ts, u32 hwnd, u32 message, u32 wParam, u32 lParam);
// The port's event (os::window::wait) given to the window's procedure as the game's message; its answer.
u32 dispatchEvent(App* app, ThreadState& ts, const os::window::Event& event);
// The game's 0x4cd350: the window created, fullscreen or framed; its handle.  With none it stays as it is; the
// arrows and such of the scan table's three keyboards are set up.
u32 createWindow(App* app, ThreadState& ts, u32 window);
// The game's 0x4cd4e0: the window's thread: makes the window, tells who waits, pumps its messages until it is
// gone.
u32 windowThread(App* app, ThreadState& ts);
// The game's 0x4cd6c0: the game's window, opened or changed for a mode: the class and the handlers the first time,
// its thread each time; its handle.
u32 openWindow(App* app, ThreadState& ts, u32 width, u32 height, u32 isFullscreen);
// The game's 0x4cdaa0: the window closed: the keys the system had put back, and waited for.
u32 closeWindow(App* app, ThreadState& ts);
// The game's 0x4cdb10: the window put at a place with a size of client area; what came kept.
u32 placeWindow(App* app, ThreadState& ts, u32 left, u32 top, u32 width, u32 height);
// The game's 0x4cdbd0: the window centred on the screen.
u32 centreWindow(App* app, ThreadState& ts, u32 width, u32 height);
// The game's 0x4cdc50: the keyboard hook off, in fullscreen.
u32 hookOff(App* app, ThreadState& ts);
// The game's 0x4cdc90: the keyboard hook on, with who is told.
u32 hookOn(App* app, ThreadState& ts, u32 alt, u32 callback);
// The game's 0x4fb470: the mouse: the place held inside the clip, the system's cursor put back when it was
// outside, the place, buttons and the click kept under the table's lock.
u32 pointerMoved(App* app, ThreadState& ts, u32 x, u32 y, u32 buttons, u32 click);
// The game's 0x4a38c0: whether the game's window is up already.
u32 alreadyRunning(App* app, ThreadState& ts);
// The game's 0x4a38f0: the file version of the game's exe, its text into the buffer given; 1 when it has one.
u32 versionText(App* app, ThreadState& ts, u32 buffer, u32 length);
// The game's 0x4b56d0: an import's answer, no more.
u32 tickCount(App* app, ThreadState& ts);
// The game's 0x4b5860: DirectX 5 and a sound or graphics library there, or the game says so in the player's
// language and leaves.
u32 checkLibrary(App* app, ThreadState& ts, u32 library, u32 language);
// The game's 0x4b5e50: a message of two tables, by the language, and the game leaves.
u32 fatalMessage(App* app, ThreadState& ts, u32 language);
// The game's 0x4b5f10: enough memory and disk for the game, or the player is asked.
u32 checkMemory(App* app, ThreadState& ts, u32 language);
// The game's 0x4b5ff0: the answer to the Direct3D device enumeration's question.
u32 deviceAnswer(App* app, ThreadState& ts, u32 word0, u32 word1, u32 word2, u32 question, u32 word4, u32 word5);
// The game's 0x4b6400: the 3D device's name and its memory from the registry; a name that says so when it has
// none.
u32 thrashDevice(App* app, ThreadState& ts, u32 buffer);
// The game's 0x4b64c0: the 3D device's description from the registry.
u32 thrashDescription(App* app, ThreadState& ts, u32 buffer);
// The game's 0x4b6520: the desktop's wallpaper setting from the registry.
u32 desktopSetting(App* app, ThreadState& ts);
// The game's 0x4b6590: what DirectX there is, 0 none, 1, 3 or 5: the port's display has all the game asks, 5.
u32 directXVersion(App* app, ThreadState& ts);
// The game's 0x4ea665: entry `index` of a table, or 0 past its end.
u32 tableEntry(App* app, ThreadState& ts, u32 table, u32 index, u32 out);
// The game's 0x4f2460: a shape's hot spots, the attachment of type 0x7c in its chain; 0 when none. A shape of a
// kind not known is said to be wrong.
u32 shapeHotspots(App* app, ThreadState& ts, u32 shape);
// The game's 0x4f87a0: the graphics' screen lock let go and its driver slot back.
u32 releaseDriverSlot(App* app, ThreadState& ts);
// The game's 0x4f9120: whether the game has the focus and a window; else sub_4f9088 says.
u32 hasFocus(App* app, ThreadState& ts);
// The game's 0x4f9360: the video buffers given back.
u32 videoClose(App* app, ThreadState& ts);
// The game's 0x4fb060: a block of memory the race kept, let go.
u32 freeKeptBlock(App* app, ThreadState& ts);
// The game's 0x4fd490: a row filled, its length halved.
u32 fillHalved(App* app, ThreadState& ts, u32 to, u32 count);
// The game's 0x4fd4b0: a row filled, the count as it is.
u32 fillSame(App* app, ThreadState& ts, u32 to, u32 count);
// The game's 0x4fd4c0: a row filled, the count doubled.
u32 fillDoubled(App* app, ThreadState& ts, u32 to, u32 count);
// The game's 0x4fd520: a row filled, the count times four.
u32 fillQuadrupled(App* app, ThreadState& ts, u32 to, u32 count);
// The game's 0x4fd4e0: 16-bit pixels of the fill colour.
u32 fillWords(App* app, ThreadState& ts, u32 upper, u32 to, u32 count);
// The game's 0x4fd500: 24-bit pixels of the fill colour.
u32 fillTriples(App* app, ThreadState& ts, u32 to, u32 count);
// The game's 0x503b00: an int shifted left into 64 bits at esi.
u32 shiftTo64(App* app, ThreadState& ts, u32 value, u32 by, u32 out);
// The game's 0x503cd0: palette entries given to the window's surface, read back unless it is in fullscreen.
u32 paletteToSurface(App* app, ThreadState& ts, u32 surface, u32 first, u32 count);
// The game's 0x503e50: palette entries set from bytes red green blue.
u32 paletteSet(App* app, ThreadState& ts, u32 first, u32 count, u32 from);
// The game's 0x503ec0: palette entries as bytes red green blue.
u32 paletteGet(App* app, ThreadState& ts, u32 first, u32 count, u32 to);
// The game's 0x50e800: the queue's first entry of a priority taken off it, 0 when it has none.
u32 queueTakeByPriority(App* app, ThreadState& ts, u32 queue, u32 wanted);
// The game's 0x517580: the graphics closed when they are open.
u32 graphicsClose(App* app, ThreadState& ts);
// The game's 0x5175b0: the system's request to close, from whichever thread.
u32 graphicsAskClose(App* app, ThreadState& ts);
// The game's 0x517920: the screen's palette, or a shape's, given to the driver.
u32 graphicsPalette(App* app, ThreadState& ts, u32 surface);
// The game's 0x519140: the video's two buffers given back through the driver's table.
u32 releaseVideoBuffers(App* app, ThreadState& ts);
// The game's 0x51a170: a surface given back through the driver's table.
u32 freeSurface(App* app, ThreadState& ts, u32 surface);
// The game's 0x5244f0: the system's tasks, the idle task of the graphics.
u32 graphicsIdle(App* app, ThreadState& ts);
// The game's 0x524500: the surface's interface asked for, or the object let go.
u32 setDirectDraw(App* app, ThreadState& ts, u32 object);
// The game's 0x524550: the graphics' lock taken or let go, as the argument says.
u32 graphicsLock(App* app, ThreadState& ts, u32 taking);
// The game's 0x5245a0: a message of the driver's: an error, on the monochrome screen, in the log, or to the
// debugger, by its level.
u32 driverMessage(App* app, ThreadState& ts, u32 level, u32 text);
// The game's 0x5245f0: the driver's event: the resize callback told on the second kind.
u32 driverEvent(App* app, ThreadState& ts, u32 kind, u32 detail);
// The game's 0x524620: the driver closed without its window, for an error.
u32 closeDriverOnly(App* app, ThreadState& ts);
// The game's 0x5247c0: the screen's size and depth, with the defaults; the graphics started when they are not.
u32 setScreenSize(App* app, ThreadState& ts, u32 widthIn, u32 heightIn, u32 depthIn);
// The game's 0x525600: what a surface keeps on the side, given back.
u32 freeSurfaceCache(App* app, ThreadState& ts, u32 surface);
// The game's 0x525630: the next byte of a text, its pointer moved.
u32 nextByte(App* app, ThreadState& ts, u32 place);
// The game's 0x525650: a character through the table of the printable ones.
u32 mapCharacter(App* app, ThreadState& ts, u32 character);
// The game's 0x525670: the next character of a text: two bytes with the top bit set, else one through the table.
u32 nextCharacter(App* app, ThreadState& ts, u32 place);
// The game's 0x52e420: the text driver started with the screen's saved two values.
u32 textStart(App* app, ThreadState& ts);

}  // namespace wina
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
