#ifndef NFS3HP_NATIVE_GRAPHICS_H
#define NFS3HP_NATIVE_GRAPHICS_H

#include "native_guest.h"

/* native_graphics.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each may call the
 * game's other functions by address on `ts`, and is false or nullopt when the game stopped in one;
 * eax as the code leaves it where its comment says so. */
namespace nfs3hp
{
namespace graphics
{

// The game's 0x431900: the render state `state` set to `value`; eax its answer.
bool sendState(nfs::App* app, nfs::ThreadState& ts, nfs::u32 state, nfs::u32 value);
// The game's 0x431900 as C++ that gives its answer back (the driver's, 1 for a state not sent); nullopt when the
// game stopped.
std::optional<nfs::u32> stateAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::u32 state, nfs::u32 value);
// The game's 0x433d40: a quad of a polygon list with colours of its own drawn.
bool colouredQuadRecord(nfs::App* app, nfs::ThreadState& ts, nfs::u32 record);
// The game's 0x4347d0: a line of a polygon list drawn.
bool lineRecord(nfs::App* app, nfs::ThreadState& ts, nfs::u32 record);
// The game's 0x482940: player's message box in the HUD, `text` in it.
bool messageBox(nfs::App* app, nfs::ThreadState& ts, nfs::u32 player, nfs::u32 text);
// The game's 0x495230: the loading bar at the per cent at `perCentAt` (the address of the word the
// code pushes; 101 written back there past 100).
bool loadingProgress(nfs::App* app, nfs::ThreadState& ts, nfs::u32 perCentAt);
// Port: whether the loading bar's last move was drawn where it was asked to go -- the bar is paced
// to take its time, and a load all done waits at 100 per cent until this says so.
bool loadingBarReached();
// The game's 0x49c600: a texture of `width` by `height` allocated; eax its handle, as the driver
// leaves it.
bool textureAllocated(nfs::App* app, nfs::ThreadState& ts, nfs::u32 width, nfs::u32 height);
// The same with the handle as the result; nullopt when the game stopped.
std::optional<nfs::u32> textureHandle(nfs::App* app, nfs::ThreadState& ts, nfs::u32 width, nfs::u32 height);
// textureAllocated's handle.
inline std::optional<nfs::u32> allocatedTexture(nfs::App* app, nfs::ThreadState& ts, nfs::u32 width,
                                                  nfs::u32 height)
{
    return textureHandle(app, ts, width, height);
}
// The game's 0x49c620: `texture` given the `width` by `height` 1555 pixels at `pixels`.
bool textureUpdated(nfs::App* app, nfs::ThreadState& ts, nfs::u32 texture, nfs::u32 pixels,
                    nfs::u32 width, nfs::u32 height);
// The game's 0x4bee80: the screen mode the Screen Size list would have for `width`, 1 for none.
std::optional<nfs::u32> screenModeFor(nfs::App* app, nfs::ThreadState& ts, nfs::u32 width);
// The game's 0x4f8830: the screen's functions kept for the driver.
bool screenFunctionsSet(nfs::App* app, nfs::ThreadState& ts);

// The game's 0x434a00: a frame ended, the picture flipped if `flip`, flushed if `flush`.
bool frameEnd(nfs::App* app, nfs::ThreadState& ts, bool flip, bool flush);
// The game's 0x434b20: the driver waited for and the window taken.
bool windowSynced(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x434b40: the picture flushed and flipped.
bool flushFlip(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x459350: a rectangle drawn flat between (left, top) and (right, bottom) in `colour`; eax what it leaves.
std::optional<nfs::u32> flatRectangle(nfs::App* app, nfs::ThreadState& ts, nfs::s32 left, nfs::s32 top,
                                        nfs::s32 right, nfs::s32 bottom, nfs::u32 colour);
// The game's 0x459440: the HUD editor's frame about `element` in `colour`.
bool editorFrame(nfs::App* app, nfs::ThreadState& ts, nfs::u32 element, nfs::u32 colour);
// The game's 0x4bebe0: the clip rectangle drawn in set to x, y, `wide` by `high`; eax what it leaves.
std::optional<nfs::u32> clipSet(nfs::App* app, nfs::ThreadState& ts, nfs::s32 x, nfs::s32 y,
                                  nfs::s32 wide, nfs::s32 high);
// The game's 0x4be570: screen mode `mode`'s width into [widthAt] and height into [heightAt]; [zeroAt] and
// [alsoZeroAt] set to 0.
bool modeSized(nfs::App* app, nfs::ThreadState& ts, nfs::u32 mode, nfs::u32 zeroAt, nfs::u32 alsoZeroAt,
               nfs::u32 widthAt, nfs::u32 heightAt);
// The game's 0x4be8e0: the quads around the view; eax what it leaves, nullopt when the game stopped.
std::optional<nfs::u32> viewSurround(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x4bed90: the screen modes listed for the handler `handler`.
bool screenModesListed(nfs::App* app, nfs::ThreadState& ts, nfs::u32 handler);
// The game's 0x4bc160: the race's frame drawn for pass `pass`.
bool raceFrameDrawn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 pass);
// The game's 0x488a80: the cabin's instruments loaded.
bool cabinInstrumentsLoaded(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x494bd0: the loading aborted.
bool loadingAborted(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x494fa0: the loading screen shown; eax as it leaves it, which is the `eax` it came with when the
// loading screen is turned off.  nullopt when the game stopped.
std::optional<nfs::u32> loadingScreenAnswer(nfs::App* app, nfs::ThreadState& ts, nfs::u32 eax);
// The game's 0x49bf60: the line mesh at `mesh` flushed; eax what it leaves.
std::optional<nfs::u32> lineMeshFlushed(nfs::App* app, nfs::ThreadState& ts, nfs::u32 mesh);
// The game's 0x4a35d0: the race taken down, sub_495600 for `extra` nonzero; eax what it leaves.
std::optional<nfs::u32> raceTakenDown(nfs::App* app, nfs::ThreadState& ts, nfs::u32 extra);
// The game's 0x4b5ea0: the texture memory warning for driver type `type`, index `index`; eax what it leaves.
std::optional<nfs::u32> textureMemoryWarned(nfs::App* app, nfs::ThreadState& ts, nfs::u32 type,
                                              nfs::u32 index);
// The game's 0x4b6250: what the game does as it ends; eax what it leaves.
std::optional<nfs::u32> gameEnded(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x4d5cc0: drawing begun, the textures reset first if `reset`, the buffers cleared if `clear`.
bool drawingBegun(nfs::App* app, nfs::ThreadState& ts, nfs::u32 reset, nfs::u32 clear);
// The game's 0x4d5d30: both buffers cleared.
bool clearTwice(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x4d5d70: both buffers cleared, then the textures reset.
bool clearTwiceAndReset(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x4d5d30 and 0x4d5d70 with eax as they leave it (the last flip's, the textures' reset's); nullopt
// when the game stopped.
std::optional<nfs::u32> clearedTwice(nfs::App* app, nfs::ThreadState& ts);
std::optional<nfs::u32> clearedTwiceAndReset(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x4d5ed0: the shape `shape` into `texture`, `size` wide at most, `limit` into the height's slot.
bool shapeIntoTextureOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 texture, nfs::u32 shape,
                        nfs::u32 size, nfs::u32 limit);
// The game's 0x4d5f40: an art texture of `size`, its format at `formatAt`, cleared if `clear`; eax its handle.
std::optional<nfs::u32> artTexture(nfs::App* app, nfs::ThreadState& ts, nfs::u32 size, nfs::u32 formatAt,
                                     nfs::u32 clear);
// The game's 0x4d9120: the window taken and cleared.
bool windowTakenAndCleared(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x4d9140: the picture flushed, flipped and waited for; eax the tick waited for.
std::optional<nfs::u32> flipAndWait(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x517170: the driver of slot `slot` taken back (its library and pointers); the slot's last word.
nfs::u32 driverSlotUsed(nfs::App* app, nfs::u32 slot);
// The game's 0x517370: the driver in use kept in slot `slot`; the slot's first word's index.
nfs::u32 driverSlotKept(nfs::App* app, nfs::u32 slot);
// The game's 0x517890: window `window` taken and the frame buffer locked for the screen record `screen`; eax 0, or 1
// with the pixels' address in edx (kept in the record), nullopt when the game stopped.
std::optional<nfs::u32> frameBufferLocked(nfs::App* app, nfs::ThreadState& ts, nfs::u32 screen,
                                            nfs::u32 window);
// The game's 0x5178d0: the frame buffer let go of.
bool frameLockReleased(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x524650: the driver closed when it is open, the game's window destroyed too for `destroyWindow`;
// eax 0 after it.
bool driverClosed(nfs::App* app, nfs::ThreadState& ts, bool destroyWindow);
// The game's 0x4c4490: the screen captured.
bool screenCaptured(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x4c5c00: the "please wait" message put up.
bool waitMessage(nfs::App* app, nfs::ThreadState& ts);

}  // namespace graphics

}  // namespace nfs3hp

#endif
