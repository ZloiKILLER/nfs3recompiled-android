#ifndef NFS3HP_NATIVE_FRONTEND_EAB_H
#define NFS3HP_NATIVE_FRONTEND_EAB_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_eab.cpp's typed functions (EA's libraries from 0x4ec870 up: DirectDraw's mode,
 * palette and locks, bitmaps and shapes, fonts and text, the low timer, memory moves, TCP and the
 * modem, the file requests, net games, memory blocks), which other files call as C++
 * (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `ts` and throws fe::Terminated
 * when the game stopped in one, as a call by address does; its result is what the game's function leaves
 * in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace eab
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x4ec870: a display (`given`, 0 the library's own) put in the mode `mode` of its table, a negative one
// 640x480x16; a window's client area made that size instead.  1 when the mode came, 0 when none is known, -1 on
// "not exclusive".
u32 ddrawSetDisplayMode(App* app, ThreadState& ts, u32 given, u32 mode);
// The game's 0x4ecd50: a display's surfaces, its DirectDraw object and its table of modes gone.
u32 ddrawDisposeDisplay(App* app, ThreadState& ts, u32 given);
// The game's 0x4ecf60: the colours changed since the last time told to the palette of the display `given`, unless a
// surface is locked (an error) or it has no palette.
u32 ddrawApplyPalette(App* app, ThreadState& ts, u32 given);
// The game's 0x4ed0f0: the surface `surface` of the display `given` locked, its bits named in `bitmap` (0 for
// none); what was locked before unlocked first.  1, or 0 when it could not be had.
u32 ddrawLockBitmap(App* app, ThreadState& ts, u32 given, u32 surface, u32 bitmap);
// The game's 0x4ed2a0: the surface the display `given` holds locked unlocked, how long it was held noted.
u32 ddrawUnlockBitmap(App* app, ThreadState& ts, u32 given);
// The game's 0x4eee30: a bitmap of `width` by `height` pixels of `bits` (0 the screen's) in the memory `flags`
// names, its rows made when the library can draw; `complain` on failure.  0 on failure.
u32 windowBitmapCreate(App* app, ThreadState& ts, u32 width, u32 height, u32 bits, u32 flags, u32 complain);
// The game's 0x4ef190: sub_4eee30, complaining as the library's flag at 0x56440c says.
u32 windowBitmapCreateDefault(App* app, ThreadState& ts, u32 width, u32 height, u32 bits, u32 flags);
// The game's 0x4ef1b0: the clip rectangle filled with `colour`.
u32 drawClearClip(App* app, ThreadState& ts, u32 colour);
// The game's 0x4ef1e0: the shape named by the four bytes at `nameAt` in the shape file `file`: last to first, the
// first entry the fallback.  None: 0, and an error if the flag at 0x56440c says to.
u32 shapeFileFind(App* app, ThreadState& ts, u32 file, u32 nameAt);
// The game's 0x4ef2b0: `pixels` 4-bit pixels from `from` to `to`, a zero one not copied.
u32 copyNibblesKeyed(App* app, ThreadState& ts, u32 from, u32 to, u32 pixels);
// The game's 0x4ef320: `count` bytes from `from` to `to`, a 0xff not copied.
u32 copyBytesKeyed(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x4ef340: `count` words from `from` to `to`, a zero one not copied.
u32 copyWordsKeyed(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x4ef370: `count` 3-byte pixels from `from` to `to`, an all-zero one not copied.
u32 copyTriplesKeyed(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x4ef3c0: `count` ARGB pixels from `from` to `to`, one with no alpha not copied.
u32 copyLongsKeyed(App* app, ThreadState& ts, u32 from, u32 to, u32 count);
// The game's 0x4ef3f0: the shape `shape` drawn at `x`, `y` with its key colour clear.
u32 shapeDrawKeyed(App* app, ThreadState& ts, u32 shape, u32 x, u32 y);
// The game's 0x4ef4c0: sub_4ef3f0 with the shape's own offset taken off the position `x`, `top`.
u32 shapeDrawKeyedAtHotspot(App* app, ThreadState& ts, u32 shape, u32 x, u32 top);
// The game's 0x4ef520: the font at `font` taken into use.
u32 fontUse(App* app, ThreadState& ts, u32 font);
// The game's 0x4ef6c0: the palette's entry nearest to `colour`, of those in use at 0x5650c4.
u32 paletteNearest(App* app, ThreadState& ts, u32 colour);
// The game's 0x4ef760: the ARGB `colour` as the screen's depth (0x565010) stores it.
u32 colourPack(App* app, ThreadState& ts, u32 colour);
// The game's 0x4ef830: the font's two colours: sixteen steps from one to the other for the shades of an
// anti-aliased glyph, packed for the screen.
u32 fontSetColours(App* app, ThreadState& ts, u32 ink, u32 back);
// The game's 0x4efa10: the record for `character` among `count` records of `stride` bytes at `table`, sorted by the
// word at each one's start; 0 if none.
u32 fontGlyphSearch(App* app, ThreadState& ts, u32 character, u32 table, u32 count, u32 stride);
// The game's 0x4efa70: the glyph of `character` in the font in use: where a table of the printable characters would
// have it, else searched for.
u32 fontGlyphFind(App* app, ThreadState& ts, u32 character);
// The game's 0x4efb10: the kerning of the pair `previous`, `character` in the font's table.
u32 fontKerning(App* app, ThreadState& ts, u32 character, u32 previous);
// The game's 0x4efb50: `text` drawn at the pen, which ends where it began: newlines, tabs, kerning, and a box for
// what has no glyph if asked.
u32 textDrawAtPen(App* app, ThreadState& ts, u32 text);
// The game's 0x4efe30: the pen moved to `x`, `y` and `text` drawn.
u32 textDrawAt(App* app, ThreadState& ts, u32 text, u32 x, u32 y);
// The game's 0x4efe50: how wide the first `limit` characters of `text` are.
u32 textMeasureUpTo(App* app, ThreadState& ts, u32 text, u32 limit);
// The game's 0x4efef0: how wide `text` is, up to 256 characters.
u32 textMeasureAll(App* app, ThreadState& ts, u32 text);
// The game's 0x4eff00: a bitmap freed: its surface if it was the screen's, its rows, its memory.
u32 windowBitmapFree(App* app, ThreadState& ts, u32 bitmap);
// The game's 0x4f00d0: the next key (sub_4cc970), its low word.
u32 keyNextLow16(App* app, ThreadState& ts);
// The game's 0x4f00e0: the next key handed on to sub_50c584.
u32 keyNextDispatch(App* app, ThreadState& ts);
// The game's 0x4f00f0: the flag at 0x5651d0 set to `flag`.
u32 keySetHookFlag(App* app, ThreadState& ts, u32 flag);
// The game's 0x4f01f0: the keys' table at 0x564818 cleared and the state marked open, with its lock made the first
// time.
u32 inputStateOpen(App* app, ThreadState& ts);
// The game's 0x4f0230: the state marked closed; eax as it was.
u32 inputStateClose(App* app, ThreadState& ts);
u32 inputStateClosedWith(App* app, ThreadState& ts, u32 eax);  // eax as the caller has it, which it leaves
// The game's 0x4f22e0: the file `name` read into `into`, its size kept at 0x5643f8, the sound topped up before:
// `into`, or 0 if empty.
u32 fileLoadInto(App* app, ThreadState& ts, u32 name, u32 into);
// The game's 0x4f24e0: a shape's record of type 0x6f (its hot spot), past the record's head; 0 if none.
u32 shapeFindHotspot(App* app, ThreadState& ts, u32 shape);
// The game's 0x4f2790: GetTickCount.
u32 lowTimerTicks(App* app, ThreadState& ts);
// The game's 0x4f27a0: the low timer's thread: at each signal of its event, the functions of the table run in turn,
// until its lock is gone.
u32 lowTimerThread(App* app, ThreadState& ts);
// The game's 0x4f2890: the low timer started: its end at exit (0x4f2830), its lock, its thread.
u32 lowTimerStart(App* app, ThreadState& ts);
// The game's 0x4f2930: `function` added to the low timer's table unless it is there; the timer started if it was
// not.
u32 lowTimerAdd(App* app, ThreadState& ts, u32 function);
// The game's 0x4f2a60: the low timer's event signalled, if it has one; else eax as it was.
// `eax` as the caller holds it: what a signal with no event gives back.
u32 lowTimerSignal(App* app, ThreadState& ts, u32 eax);
// The game's 0x4f2a78: the runtime's atexit for `function` (a jmp).
u32 atexitJump(App* app, ThreadState& ts, u32 function);
// The game's 0x4f2a7d: exit(0): it does not come back.
u32 exitGameNow(App* app, ThreadState& ts);
// The game's 0x4f2ad0: memcpy of `size` bytes from `from` to `to`, `lead` of them by bytes to align it, a Duff's
// device on 16 words, the last bytes one at a time; the registers as the device leaves them.
u32 memCopyAligned(App* app, ThreadState& ts, u32 from, u32 to, u32 size, u32 lead);
// The game's 0x4f2bf0: memmove of `size` bytes from `from` to `to`: from the end when they overlap or are few, by
// sub_4f2ad0 else.
u32 memMoveLibrary(App* app, ThreadState& ts, u32 from, u32 to, u32 size);
// The game's 0x4f2c40: sub_4f2bf0 from `from` to `to`, `count` half bytes.
u32 memMoveHalfBytes(App* app, ThreadState& ts, u32 to, u32 from, u32 count);
// The game's 0x4f2c50: sub_4f2bf0 from `from` to `to`, `count` bytes.
u32 memMoveBytes(App* app, ThreadState& ts, u32 to, u32 from, u32 count);
// The game's 0x4f2c60: sub_4f2bf0 from `from` to `to`, `count` words.
u32 memMoveWords(App* app, ThreadState& ts, u32 to, u32 from, u32 count);
// The game's 0x4f2c70: sub_4f2bf0 from `from` to `to`, `count` three-byte pixels.
u32 memMoveTriples(App* app, ThreadState& ts, u32 to, u32 from, u32 count);
// The game's 0x4f2c90: sub_4f2bf0 from `from` to `to`, `count` longs.
u32 memMoveLongs(App* app, ThreadState& ts, u32 to, u32 from, u32 count);
// The game's 0x4f2cb0: the shape `shape` moved to `x`, `y` in memory, the place's low twelve bits of each kept in
// its fields at 0xc and 0xe.
u32 shapeMoveWithSpot(App* app, ThreadState& ts, u32 shape, u32 x, u32 y);
// The game's 0x4f7450: a listening socket on `port` for `callback` and `argument`: its record, its thread waited
// for till it runs or gives up.  The record, or 0.
u32 tcpOpen(App* app, ThreadState& ts, u32 port, u32 callback, u32 argument);
// The game's 0x4f75d0: a socket's record closed: its thread told and waited for, the socket, the memory.
u32 tcpClose(App* app, ThreadState& ts, u32 record);
// The game's 0x4f7660: the first field of a record.
u32 tcpFirstField(App* app, ThreadState& ts, u32 record);
// The game's 0x4f7670: `name` made an address at `address` on `port`, or on the word at `portAt` if that is 0: a
// dotted number or a host's.  1, or 0.
u32 tcpResolve(App* app, ThreadState& ts, u32 portAt, u32 name, u32 port, u32 address);
// The game's 0x4f7750: an address as text at `text`, its port through `portOut` if asked for.
u32 tcpAddressText(App* app, ThreadState& ts, u32 address, u32 text, u32 portOut);
// The game's 0x4f77c0: whether the 16 bytes at `a` and at `b` are the same.
u32 tcpSameAddress(App* app, ThreadState& ts, u32 a, u32 b);
// The game's 0x4f77f0: the socket's own address (getsockname) into `out`.
u32 tcpLocalName(App* app, ThreadState& ts, u32 record, u32 out);
// The game's 0x4f9088: the end of sub_4f9070, called on its own: 0.
u32 lineNoFlag(App* app, ThreadState& ts);
// The game's 0x4f9500: TAPI's callback (stdcall): each message told to the log; a call's state answered or dropped,
// its end the result at 0x9f4d9c.
u32 lineCallback(App* app, ThreadState& ts, u32 device, u32 what, u32 instance, u32 p1, u32 p2, u32 p3);
// The game's 0x4f97c0: the modem shut: a second's wait, the call dropped, the line closed, the reading thread or
// its handle gone, the buffers freed and TAPI told.
u32 lineShutdown(App* app, ThreadState& ts);
// The game's 0x4fbc30: a file `name` opened as `mode` asks; a name with a '|' is a pack and a member, looked for
// among the open packs, as is one that did not open when `search` says.
u32 asyncOpenFile(App* app, ThreadState& ts, u32 name, u32 mode, u32 search);
// The game's 0x4fbda0: a thread of the file requests: a request of the highest priority it may take from its queue,
// served; put on the queue of what is done, its callback told.
u32 asyncFileThread(App* app, ThreadState& ts);
// The game's 0x4fc170: the file requests ended, at exit: what is open closed, each thread woken and waited for, its
// events and queues gone, the memory and the lock.
// `eax` as the caller holds it: what a shutdown with no threads gives back.
u32 asyncFilesShutdown(App* app, ThreadState& ts, u32 eax);
// The game's 0x4fc250: the file requests set up, once: `handles` handles (0: 0x40), `records` records (0: 0x40) on
// the free queue, room for the threads, the lock, the end at exit.
u32 asyncFilesInit(App* app, ThreadState& ts, u32 handles, u32 records);
// The game's 0x514280: a game `net` on a transport opened, `opening` the argument of the transport's opening; its
// `address`, `name` and `password` kept.  0xfa, or 0.
u32 netOpen(App* app, ThreadState& ts, u32 net, u32 address, u32 opening, u32 name, u32 password);
// The game's 0x514320: a game's transport closed: the game ended, what it was waiting for done; the last one stops
// the timer and the transport.
u32 netClose(App* app, ThreadState& ts, u32 net);
// The game's 0x5150f0: the machine's own address as text at `out`: from the network's adapters, else its host
// name's.
u32 netLocalAddress(App* app, ThreadState& ts, u32 out);
// The game's 0x517bf0: the blocks of the pool `which` written out: name, size, room, owner, checksum, kind, and the
// total of what is free.
u32 memPoolDump(App* app, ThreadState& ts, u32 which);
// The game's 0x517d60: the dump of the library's pool now.
u32 memPoolDumpDefault(App* app, ThreadState& ts);
// The game's 0x517e50: whether `block` ends with the "CRCF" tag of what was loaded with a checksum.
u32 blockHasCrcTag(App* app, ThreadState& ts, u32 block);
// The game's 0x517eb0: whether the checksum of `block` holds; one with no tag does.
u32 blockCrcGood(App* app, ThreadState& ts, u32 block);
// The game's 0x517f10: `block` checked as `name` names it: no tag or a bad sum an error if asked for and 0; a good
// one has its tail cut off.  The block.
u32 blockCheckCrc(App* app, ThreadState& ts, u32 block, u32 name);
// The game's 0x524bb0: the question of leaving (Y/N) over the screen, the game left on a yes; the screen let go
// first if it is held.
u32 exitConfirm(App* app, ThreadState& ts);
// The game's 0x524df0: `block` made `size` bytes in place, as far as there is room up to the next: -1 as much as
// can be; what is past made a free block if it is worth it.
u32 blockResize(App* app, ThreadState& ts, u32 block, u32 size);
// The game's 0x4ea440: how many files the BIG archive `archive` holds, by its kind (sub_4e9f00).
u32 bigFileCount(App* app, ThreadState& ts, u32 archive);
// The game's 0x4ea4a0: the name of file `index` of the BIG archive `archive` copied to `out`, or an empty name for
// none: the copy, or 0.
u32 bigFileName(App* app, ThreadState& ts, u32 archive, u32 index, u32 out);

}  // namespace eab
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
