#ifndef NFS3HP_NATIVE_FRONTEND_EAB_H
#define NFS3HP_NATIVE_FRONTEND_EAB_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_eab.cpp's typed functions (EA's libraries from 0x4ec870 up: DirectDraw's mode,
 * palette and locks, bitmaps and shapes, fonts and text, the low timer, memory moves, TCP and the
 * modem, the file requests, net games, memory blocks), which other files call as C++
 * (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `cpu` and throws fe::Terminated
 * when the game stopped in one, as a call by address does; its result is what the game's function leaves
 * in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace eab
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x4ec870: a display (`given`, 0 the library's own) put in the mode `mode` of its table, a negative one
// 640x480x16; a window's client area made that size instead.  1 when the mode came, 0 when none is known, -1 on
// "not exclusive".
reg32 ddrawSetDisplayMode(App* app, CPU& cpu, reg32 given, reg32 mode);
// The game's 0x4ecd50: a display's surfaces, its DirectDraw object and its table of modes gone.
reg32 ddrawDisposeDisplay(App* app, CPU& cpu, reg32 given);
// The game's 0x4ecf60: the colours changed since the last time told to the palette of the display `given`, unless a
// surface is locked (an error) or it has no palette.
reg32 ddrawApplyPalette(App* app, CPU& cpu, reg32 given);
// The game's 0x4ed0f0: the surface `surface` of the display `given` locked, its bits named in `bitmap` (0 for
// none); what was locked before unlocked first.  1, or 0 when it could not be had.
reg32 ddrawLockBitmap(App* app, CPU& cpu, reg32 given, reg32 surface, reg32 bitmap);
// The game's 0x4ed2a0: the surface the display `given` holds locked unlocked, how long it was held noted.
reg32 ddrawUnlockBitmap(App* app, CPU& cpu, reg32 given);
// The game's 0x4eee30: a bitmap of `width` by `height` pixels of `bits` (0 the screen's) in the memory `flags`
// names, its rows made when the library can draw; `complain` on failure.  0 on failure.
reg32 windowBitmapCreate(App* app, CPU& cpu, reg32 width, reg32 height, reg32 bits, reg32 flags, reg32 complain);
// The game's 0x4ef190: sub_4eee30, complaining as the library's flag at 0x56440c says.
reg32 windowBitmapCreateDefault(App* app, CPU& cpu, reg32 width, reg32 height, reg32 bits, reg32 flags);
// The game's 0x4ef1b0: the clip rectangle filled with `colour`.
reg32 drawClearClip(App* app, CPU& cpu, reg32 colour);
// The game's 0x4ef1e0: the shape named by the four bytes at `nameAt` in the shape file `file`: last to first, the
// first entry the fallback.  None: 0, and an error if the flag at 0x56440c says to.
reg32 shapeFileFind(App* app, CPU& cpu, reg32 file, reg32 nameAt);
// The game's 0x4ef2b0: `pixels` 4-bit pixels from `from` to `to`, a zero one not copied.
reg32 copyNibblesKeyed(App* app, CPU& cpu, reg32 from, reg32 to, reg32 pixels);
// The game's 0x4ef320: `count` bytes from `from` to `to`, a 0xff not copied.
reg32 copyBytesKeyed(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x4ef340: `count` words from `from` to `to`, a zero one not copied.
reg32 copyWordsKeyed(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x4ef370: `count` 3-byte pixels from `from` to `to`, an all-zero one not copied.
reg32 copyTriplesKeyed(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x4ef3c0: `count` ARGB pixels from `from` to `to`, one with no alpha not copied.
reg32 copyLongsKeyed(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count);
// The game's 0x4ef3f0: the shape `shape` drawn at `x`, `y` with its key colour clear.
reg32 shapeDrawKeyed(App* app, CPU& cpu, reg32 shape, reg32 x, reg32 y);
// The game's 0x4ef4c0: sub_4ef3f0 with the shape's own offset taken off the position `x`, `top`.
reg32 shapeDrawKeyedAtHotspot(App* app, CPU& cpu, reg32 shape, reg32 x, reg32 top);
// The game's 0x4ef520: the font at `font` taken into use.
reg32 fontUse(App* app, CPU& cpu, reg32 font);
// The game's 0x4ef6c0: the palette's entry nearest to `colour`, of those in use at 0x5650c4.
reg32 paletteNearest(App* app, CPU& cpu, reg32 colour);
// The game's 0x4ef760: the ARGB `colour` as the screen's depth (0x565010) stores it.
reg32 colourPack(App* app, CPU& cpu, reg32 colour);
// The game's 0x4ef830: the font's two colours: sixteen steps from one to the other for the shades of an
// anti-aliased glyph, packed for the screen.
reg32 fontSetColours(App* app, CPU& cpu, reg32 ink, reg32 back);
// The game's 0x4efa10: the record for `character` among `count` records of `stride` bytes at `table`, sorted by the
// word at each one's start; 0 if none.
reg32 fontGlyphSearch(App* app, CPU& cpu, reg32 character, reg32 table, reg32 count, reg32 stride);
// The game's 0x4efa70: the glyph of `character` in the font in use: where a table of the printable characters would
// have it, else searched for.
reg32 fontGlyphFind(App* app, CPU& cpu, reg32 character);
// The game's 0x4efb10: the kerning of the pair `previous`, `character` in the font's table.
reg32 fontKerning(App* app, CPU& cpu, reg32 character, reg32 previous);
// The game's 0x4efb50: `text` drawn at the pen, which ends where it began: newlines, tabs, kerning, and a box for
// what has no glyph if asked.
reg32 textDrawAtPen(App* app, CPU& cpu, reg32 text);
// The game's 0x4efe30: the pen moved to `x`, `y` and `text` drawn.
reg32 textDrawAt(App* app, CPU& cpu, reg32 text, reg32 x, reg32 y);
// The game's 0x4efe50: how wide the first `limit` characters of `text` are.
reg32 textMeasureUpTo(App* app, CPU& cpu, reg32 text, reg32 limit);
// The game's 0x4efef0: how wide `text` is, up to 256 characters.
reg32 textMeasureAll(App* app, CPU& cpu, reg32 text);
// The game's 0x4eff00: a bitmap freed: its surface if it was the screen's, its rows, its memory.
reg32 windowBitmapFree(App* app, CPU& cpu, reg32 bitmap);
// The game's 0x4f00d0: the next key (sub_4cc970), its low word.
reg32 keyNextLow16(App* app, CPU& cpu);
// The game's 0x4f00e0: the next key handed on to sub_50c584.
reg32 keyNextDispatch(App* app, CPU& cpu);
// The game's 0x4f00f0: the flag at 0x5651d0 set to `flag`.
reg32 keySetHookFlag(App* app, CPU& cpu, reg32 flag);
// The game's 0x4f01f0: the keys' table at 0x564818 cleared and the state marked open, with its lock made the first
// time.
reg32 inputStateOpen(App* app, CPU& cpu);
// The game's 0x4f0230: the state marked closed; eax as it was.
reg32 inputStateClose(App* app, CPU& cpu);
reg32 inputStateClosedWith(App* app, CPU& cpu, reg32 eax);  // eax as the caller has it, which it leaves
// The game's 0x4f22e0: the file `name` read into `into`, its size kept at 0x5643f8, the sound topped up before:
// `into`, or 0 if empty.
reg32 fileLoadInto(App* app, CPU& cpu, reg32 name, reg32 into);
// The game's 0x4f24e0: a shape's record of type 0x6f (its hot spot), past the record's head; 0 if none.
reg32 shapeFindHotspot(App* app, CPU& cpu, reg32 shape);
// The game's 0x4f2790: GetTickCount.
reg32 lowTimerTicks(App* app, CPU& cpu);
// The game's 0x4f27a0: the low timer's thread: at each signal of its event, the functions of the table run in turn,
// until its lock is gone.
reg32 lowTimerThread(App* app, CPU& cpu);
// The game's 0x4f2890: the low timer started: its end at exit (0x4f2830), its lock, its thread.
reg32 lowTimerStart(App* app, CPU& cpu);
// The game's 0x4f2930: `function` added to the low timer's table unless it is there; the timer started if it was
// not.
reg32 lowTimerAdd(App* app, CPU& cpu, reg32 function);
// The game's 0x4f2a60: the low timer's event signalled, if it has one; else eax as it was.
// `eax` as the caller holds it: what a signal with no event gives back.
reg32 lowTimerSignal(App* app, CPU& cpu, reg32 eax);
// The game's 0x4f2a78: the runtime's atexit for `function` (a jmp).
reg32 atexitJump(App* app, CPU& cpu, reg32 function);
// The game's 0x4f2a7d: exit(0): it does not come back.
reg32 exitGameNow(App* app, CPU& cpu);
// The game's 0x4f2ad0: memcpy of `size` bytes from `from` to `to`, `lead` of them by bytes to align it, a Duff's
// device on 16 words, the last bytes one at a time; the registers as the device leaves them.
reg32 memCopyAligned(App* app, CPU& cpu, reg32 from, reg32 to, reg32 size, reg32 lead);
// The game's 0x4f2bf0: memmove of `size` bytes from `from` to `to`: from the end when they overlap or are few, by
// sub_4f2ad0 else.
reg32 memMoveLibrary(App* app, CPU& cpu, reg32 from, reg32 to, reg32 size);
// The game's 0x4f2c40: sub_4f2bf0 from `from` to `to`, `count` half bytes.
reg32 memMoveHalfBytes(App* app, CPU& cpu, reg32 to, reg32 from, reg32 count);
// The game's 0x4f2c50: sub_4f2bf0 from `from` to `to`, `count` bytes.
reg32 memMoveBytes(App* app, CPU& cpu, reg32 to, reg32 from, reg32 count);
// The game's 0x4f2c60: sub_4f2bf0 from `from` to `to`, `count` words.
reg32 memMoveWords(App* app, CPU& cpu, reg32 to, reg32 from, reg32 count);
// The game's 0x4f2c70: sub_4f2bf0 from `from` to `to`, `count` three-byte pixels.
reg32 memMoveTriples(App* app, CPU& cpu, reg32 to, reg32 from, reg32 count);
// The game's 0x4f2c90: sub_4f2bf0 from `from` to `to`, `count` longs.
reg32 memMoveLongs(App* app, CPU& cpu, reg32 to, reg32 from, reg32 count);
// The game's 0x4f2cb0: the shape `shape` moved to `x`, `y` in memory, the place's low twelve bits of each kept in
// its fields at 0xc and 0xe.
reg32 shapeMoveWithSpot(App* app, CPU& cpu, reg32 shape, reg32 x, reg32 y);
// The game's 0x4f7450: a listening socket on `port` for `callback` and `argument`: its record, its thread waited
// for till it runs or gives up.  The record, or 0.
reg32 tcpOpen(App* app, CPU& cpu, reg32 port, reg32 callback, reg32 argument);
// The game's 0x4f75d0: a socket's record closed: its thread told and waited for, the socket, the memory.
reg32 tcpClose(App* app, CPU& cpu, reg32 record);
// The game's 0x4f7660: the first field of a record.
reg32 tcpFirstField(App* app, CPU& cpu, reg32 record);
// The game's 0x4f7670: `name` made an address at `address` on `port`, or on the word at `portAt` if that is 0: a
// dotted number or a host's.  1, or 0.
reg32 tcpResolve(App* app, CPU& cpu, reg32 portAt, reg32 name, reg32 port, reg32 address);
// The game's 0x4f7750: an address as text at `text`, its port through `portOut` if asked for.
reg32 tcpAddressText(App* app, CPU& cpu, reg32 address, reg32 text, reg32 portOut);
// The game's 0x4f77c0: whether the 16 bytes at `a` and at `b` are the same.
reg32 tcpSameAddress(App* app, CPU& cpu, reg32 a, reg32 b);
// The game's 0x4f77f0: the socket's own address (getsockname) into `out`.
reg32 tcpLocalName(App* app, CPU& cpu, reg32 record, reg32 out);
// The game's 0x4f9088: the end of sub_4f9070, called on its own: 0.
reg32 lineNoFlag(App* app, CPU& cpu);
// The game's 0x4f9500: TAPI's callback (stdcall): each message told to the log; a call's state answered or dropped,
// its end the result at 0x9f4d9c.
reg32 lineCallback(App* app, CPU& cpu, reg32 device, reg32 what, reg32 instance, reg32 p1, reg32 p2, reg32 p3);
// The game's 0x4f97c0: the modem shut: a second's wait, the call dropped, the line closed, the reading thread or
// its handle gone, the buffers freed and TAPI told.
reg32 lineShutdown(App* app, CPU& cpu);
// The game's 0x4fbc30: a file `name` opened as `mode` asks; a name with a '|' is a pack and a member, looked for
// among the open packs, as is one that did not open when `search` says.
reg32 asyncOpenFile(App* app, CPU& cpu, reg32 name, reg32 mode, reg32 search);
// The game's 0x4fbda0: a thread of the file requests: a request of the highest priority it may take from its queue,
// served; put on the queue of what is done, its callback told.
reg32 asyncFileThread(App* app, CPU& cpu);
// The game's 0x4fc170: the file requests ended, at exit: what is open closed, each thread woken and waited for, its
// events and queues gone, the memory and the lock.
// `eax` as the caller holds it: what a shutdown with no threads gives back.
reg32 asyncFilesShutdown(App* app, CPU& cpu, reg32 eax);
// The game's 0x4fc250: the file requests set up, once: `handles` handles (0: 0x40), `records` records (0: 0x40) on
// the free queue, room for the threads, the lock, the end at exit.
reg32 asyncFilesInit(App* app, CPU& cpu, reg32 handles, reg32 records);
// The game's 0x514280: a game `net` on a transport opened, `opening` the argument of the transport's opening; its
// `address`, `name` and `password` kept.  0xfa, or 0.
reg32 netOpen(App* app, CPU& cpu, reg32 net, reg32 address, reg32 opening, reg32 name, reg32 password);
// The game's 0x514320: a game's transport closed: the game ended, what it was waiting for done; the last one stops
// the timer and the transport.
reg32 netClose(App* app, CPU& cpu, reg32 net);
// The game's 0x5150f0: the machine's own address as text at `out`: from the network's adapters, else its host
// name's.
reg32 netLocalAddress(App* app, CPU& cpu, reg32 out);
// The game's 0x517bf0: the blocks of the pool `which` written out: name, size, room, owner, checksum, kind, and the
// total of what is free.
reg32 memPoolDump(App* app, CPU& cpu, reg32 which);
// The game's 0x517d60: the dump of the library's pool now.
reg32 memPoolDumpDefault(App* app, CPU& cpu);
// The game's 0x517e50: whether `block` ends with the "CRCF" tag of what was loaded with a checksum.
reg32 blockHasCrcTag(App* app, CPU& cpu, reg32 block);
// The game's 0x517eb0: whether the checksum of `block` holds; one with no tag does.
reg32 blockCrcGood(App* app, CPU& cpu, reg32 block);
// The game's 0x517f10: `block` checked as `name` names it: no tag or a bad sum an error if asked for and 0; a good
// one has its tail cut off.  The block.
reg32 blockCheckCrc(App* app, CPU& cpu, reg32 block, reg32 name);
// The game's 0x524bb0: the question of leaving (Y/N) over the screen, the game left on a yes; the screen let go
// first if it is held.
reg32 exitConfirm(App* app, CPU& cpu);
// The game's 0x524df0: `block` made `size` bytes in place, as far as there is room up to the next: -1 as much as
// can be; what is past made a free block if it is worth it.
reg32 blockResize(App* app, CPU& cpu, reg32 block, reg32 size);
// The game's 0x4ea440: how many files the BIG archive `archive` holds, by its kind (sub_4e9f00).
reg32 bigFileCount(App* app, CPU& cpu, reg32 archive);
// The game's 0x4ea4a0: the name of file `index` of the BIG archive `archive` copied to `out`, or an empty name for
// none: the copy, or 0.
reg32 bigFileName(App* app, CPU& cpu, reg32 archive, reg32 index, reg32 out);

}  // namespace eab
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
