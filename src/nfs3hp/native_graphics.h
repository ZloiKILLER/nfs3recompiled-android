#ifndef NFS3HP_NATIVE_GRAPHICS_H
#define NFS3HP_NATIVE_GRAPHICS_H

#include "native_guest.h"

/* native_graphics.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each may call the
 * game's other functions by address on `cpu`, and is false or nullopt when the game stopped in one;
 * eax as the code leaves it where its comment says so. */
namespace nfs3hp
{
namespace graphics
{

// The game's 0x431900: the render state `state` set to `value`; eax its answer.
bool sendState(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 state, x86::reg32 value);
// The game's 0x431900 as C++ that gives its answer back (the driver's, 1 for a state not sent); nullopt when the
// game stopped.
std::optional<x86::reg32> stateAnswer(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 state, x86::reg32 value);
// The game's 0x432390: the software blending's rows for the colour 0xAARRGGBB chosen and sent.
bool softwareBlendColour(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 colour);
// The game's 0x433d40: a quad of a polygon list with colours of its own drawn.
bool colouredQuadRecord(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 record);
// The game's 0x4347d0: a line of a polygon list drawn.
bool lineRecord(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 record);
// The game's 0x482940: player's message box in the HUD, `text` in it.
bool messageBox(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player, x86::reg32 text);
// The game's 0x495230: the loading bar at the per cent at `perCentAt` (the address of the word the
// code pushes; 101 written back there past 100).
bool loadingProgress(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 perCentAt);
// The game's 0x49c600: a texture of `width` by `height` allocated; eax its handle, as the driver
// leaves it.
bool textureAllocated(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 width, x86::reg32 height);
// The same with the handle as the result; nullopt when the game stopped.
std::optional<x86::reg32> textureHandle(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 width, x86::reg32 height);
// textureAllocated's handle.
inline std::optional<x86::reg32> allocatedTexture(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 width,
                                                  x86::reg32 height)
{
    return textureHandle(app, cpu, width, height);
}
// The game's 0x49c620: `texture` given the `width` by `height` 1555 pixels at `pixels`.
bool textureUpdated(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 texture, x86::reg32 pixels,
                    x86::reg32 width, x86::reg32 height);
// The game's 0x4bee80: the screen mode the Screen Size list would have for `width`, 1 for none.
std::optional<x86::reg32> screenModeFor(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 width);
// The game's 0x4eb3c0: the table of reciprocals made, once.
bool reciprocalsMade(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x4f8830: the screen's functions kept for the driver.
bool screenFunctionsSet(win32::WinApplication* app, x86::CPU& cpu);

// The game's 0x434a00: a frame ended, the picture flipped if `flip`, flushed if `flush`.
bool frameEnd(win32::WinApplication* app, x86::CPU& cpu, bool flip, bool flush);
// The game's 0x434b20: the driver waited for and the window taken.
bool windowSynced(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x434b40: the picture flushed and flipped.
bool flushFlip(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x459350: a rectangle drawn flat between (left, top) and (right, bottom) in `colour`; eax what it leaves.
std::optional<x86::reg32> flatRectangle(win32::WinApplication* app, x86::CPU& cpu, x86::sreg32 left, x86::sreg32 top,
                                        x86::sreg32 right, x86::sreg32 bottom, x86::reg32 colour);
// The game's 0x459440: the HUD editor's frame about `element` in `colour`.
bool editorFrame(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 element, x86::reg32 colour);
// The game's 0x4bebe0: the clip rectangle drawn in set to x, y, `wide` by `high`; eax what it leaves.
std::optional<x86::reg32> clipSet(win32::WinApplication* app, x86::CPU& cpu, x86::sreg32 x, x86::sreg32 y,
                                  x86::sreg32 wide, x86::sreg32 high);
// The game's 0x4be570: screen mode `mode`'s width into [widthAt] and height into [heightAt]; [zeroAt] and
// [alsoZeroAt] set to 0.
bool modeSized(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 mode, x86::reg32 zeroAt, x86::reg32 alsoZeroAt,
               x86::reg32 widthAt, x86::reg32 heightAt);
// The game's 0x4be8e0: the quads around the view; eax what it leaves, nullopt when the game stopped.
std::optional<x86::reg32> viewSurround(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x4bed90: the screen modes listed for the handler `handler`.
bool screenModesListed(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 handler);
// The game's 0x4bc160: the race's frame drawn for pass `pass`.
bool raceFrameDrawn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 pass);
// The game's 0x488a80: the cabin's instruments loaded.
bool cabinInstrumentsLoaded(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x494bd0: the loading aborted.
bool loadingAborted(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x494fa0: the loading screen shown; eax as it leaves it, which is the `eax` it came with when the
// loading screen is turned off.  nullopt when the game stopped.
std::optional<x86::reg32> loadingScreenAnswer(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 eax);
// The game's 0x49bf60: the line mesh at `mesh` flushed; eax what it leaves.
std::optional<x86::reg32> lineMeshFlushed(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 mesh);
// The game's 0x4a35d0: the race taken down, sub_495600 for `extra` nonzero; eax what it leaves.
std::optional<x86::reg32> raceTakenDown(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 extra);
// The game's 0x4b5ea0: the texture memory warning for driver type `type`, index `index`; eax what it leaves.
std::optional<x86::reg32> textureMemoryWarned(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 type,
                                              x86::reg32 index);
// The game's 0x4b6250: what the game does as it ends; eax what it leaves.
std::optional<x86::reg32> gameEnded(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x4d5cc0: drawing begun, the textures reset first if `reset`, the buffers cleared if `clear`.
bool drawingBegun(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 reset, x86::reg32 clear);
// The game's 0x4d5d30: both buffers cleared.
bool clearTwice(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x4d5d70: both buffers cleared, then the textures reset.
bool clearTwiceAndReset(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x4d5d30 and 0x4d5d70 with eax as they leave it (the last flip's, the textures' reset's); nullopt
// when the game stopped.
std::optional<x86::reg32> clearedTwice(win32::WinApplication* app, x86::CPU& cpu);
std::optional<x86::reg32> clearedTwiceAndReset(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x4d5ed0: the shape `shape` into `texture`, `size` wide at most, `limit` into the height's slot.
bool shapeIntoTextureOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 texture, x86::reg32 shape,
                        x86::reg32 size, x86::reg32 limit);
// The game's 0x4d5f40: an art texture of `size`, its format at `formatAt`, cleared if `clear`; eax its handle.
std::optional<x86::reg32> artTexture(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 size, x86::reg32 formatAt,
                                     x86::reg32 clear);
// The game's 0x4d9120: the window taken and cleared.
bool windowTakenAndCleared(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x4d9140: the picture flushed, flipped and waited for; eax the tick waited for.
std::optional<x86::reg32> flipAndWait(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x517170: the driver of slot `slot` taken back (its library and pointers); the slot's last word.
x86::reg32 driverSlotUsed(win32::WinApplication* app, x86::reg32 slot);
// The game's 0x517370: the driver in use kept in slot `slot`; the slot's first word's index.
x86::reg32 driverSlotKept(win32::WinApplication* app, x86::reg32 slot);
// The game's 0x517890: window `window` taken and the frame buffer locked for the screen record `screen`; eax 0, or 1
// with the pixels' address in edx (kept in the record), nullopt when the game stopped.
std::optional<x86::reg32> frameBufferLocked(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 screen,
                                            x86::reg32 window);
// The game's 0x5178d0: the frame buffer let go of.
bool frameLockReleased(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x524650: the driver closed when it is open, the game's window destroyed too for `destroyWindow`;
// eax 0 after it.
bool driverClosed(win32::WinApplication* app, x86::CPU& cpu, bool destroyWindow);
// The game's 0x4c4490: the screen captured.
bool screenCaptured(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x4c5c00: the "please wait" message put up.
bool waitMessage(win32::WinApplication* app, x86::CPU& cpu);

}  // namespace graphics

}  // namespace nfs3hp

#endif
